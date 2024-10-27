#include "httplib.h"
#include <aws/core/Aws.h>
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/PutItemRequest.h>
#include <aws/dynamodb/model/AttributeValue.h>
#include <aws/dynamodb/model/GetItemRequest.h>
#include <aws/dynamodb/model/DeleteItemRequest.h>
#include <iostream>
#include <string>
#include <ctime>
using namespace std;

class Vehicle {
protected:
    string model;
    float pricePerMinute;
public:
    Vehicle(string m, float p) : model(m), pricePerMinute(p) {}

    virtual void showDetails() {
        cout << "Model: " << model << endl;
        cout << "Price per Minute: " << pricePerMinute << " Rs" << endl;
    }

    string getModel() {
        return model;
    }

    float getPricePerMinute() {
        return pricePerMinute;
    }

    virtual ~Vehicle() {}
};


class Luxury : public Vehicle {
public:
    Luxury() : Vehicle("Luxury", 2499.0) {}
};

class MidSection : public Vehicle {
public:
    MidSection() : Vehicle("Mid-Section", 1499.0) {}
};

class LowCategory : public Vehicle {
public:
    LowCategory() : Vehicle("Low Category", 799.0) {}
};


Vehicle* createVehicleFromModel(const string& model) {
    if (model == "Luxury") {
        return new Luxury();
    } else if (model == "Mid-Section") {
        return new MidSection();
    } else if (model == "Low Category") {
        return new LowCategory();
    } else {
        return nullptr;
    }
}


class RentalSystem {
    Vehicle* vehicle;
    Aws::DynamoDB::DynamoDBClient& dynamoClient;

public:
    RentalSystem(Aws::DynamoDB::DynamoDBClient& client) : dynamoClient(client), vehicle(nullptr) {}
      int totalCost = 0;
   
    string bookCar(const string& userID, const string& userName, int choice, int rentalDuration) {
        string message;
        switch (choice) {
            case 1:
                vehicle = new Luxury();
                break;
            case 2:
                vehicle = new MidSection();
                break;
            case 3:
                vehicle = new LowCategory();
                break;
            default:
                return "Invalid choice!";
        }

        time_t startTime = time(0); 

        Aws::DynamoDB::Model::PutItemRequest putRequest;
        putRequest.SetTableName("CAR_RENTAL");

        Aws::DynamoDB::Model::AttributeValue idAttr, nameAttr, modelAttr, durationAttr, startTimeAttr;
        idAttr.SetS(userID);
        nameAttr.SetS(userName);
        modelAttr.SetS(vehicle->getModel());
        durationAttr.SetN(std::to_string(rentalDuration));
        startTimeAttr.SetN(std::to_string(startTime)); 

        putRequest.AddItem("CAR_RENTAL", idAttr);
        putRequest.AddItem("Name", nameAttr);
        putRequest.AddItem("Model", modelAttr);
        putRequest.AddItem("Duration", durationAttr);
        putRequest.AddItem("StartTime", startTimeAttr); 

        auto putOutcome = dynamoClient.PutItem(putRequest);
        if (!putOutcome.IsSuccess()) {
            message = "Failed to book car: " + putOutcome.GetError().GetMessage();
        } else {
            message = "Car booked successfully for " + userName + " with duration: " + std::to_string(rentalDuration) + " minutes.";
        }
        return message;
    }

   
    string returnCar(const string& userID) {
        string message;
        Aws::DynamoDB::Model::GetItemRequest getRequest;
        getRequest.SetTableName("CAR_RENTAL");

        Aws::DynamoDB::Model::AttributeValue keyAttr;
        keyAttr.SetS(userID);
        getRequest.AddKey("CAR_RENTAL", keyAttr);

        auto getOutcome = dynamoClient.GetItem(getRequest);
        if (!getOutcome.IsSuccess()) {
            return "Failed to retrieve user details: " + getOutcome.GetError().GetMessage();
        }

        const auto& item = getOutcome.GetResult().GetItem();
        if (item.find("Name") == item.end()) {
            return "User not found!";
        }

        string model = item.at("Model").GetS();
        int duration = stoi(item.at("Duration").GetN());
        time_t startTime = stoi(item.at("StartTime").GetN());

        vehicle = createVehicleFromModel(model);
        time_t currentTime = time(0); 
        double elapsedMinutes = difftime(currentTime, startTime) / 60;

        
        if (elapsedMinutes > duration) {
            float lateMinutes = elapsedMinutes - duration;
            int lateFee = lateMinutes * vehicle->getPricePerMinute() + 0.05;
            totalCost = duration * vehicle->getPricePerMinute() + lateFee;
            message = "Late return! Total cost: " + std::to_string(totalCost) + " Rs (including late fee).";
        } else {
            totalCost = duration * vehicle->getPricePerMinute();
            message = "Thank you for returning on time! Total cost: " + std::to_string(totalCost) + " Rs.";
        }

      
        Aws::DynamoDB::Model::DeleteItemRequest deleteRequest;
        deleteRequest.SetTableName("CAR_RENTAL");
        deleteRequest.AddKey("CAR_RENTAL", keyAttr);

        auto deleteOutcome = dynamoClient.DeleteItem(deleteRequest);
        if (!deleteOutcome.IsSuccess()) {
            return "Failed to delete user record: " + deleteOutcome.GetError().GetMessage();
        }

        return message;
    }

    ~RentalSystem() {
        delete vehicle;
    }
};

string generate_html(const std::string& title, const std::string& content) {
    return R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>)" + title + R"(</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                display: flex;
                flex-direction: column;
                align-items: center;
                justify-content: center;
                height: 100vh;
                background-color: #f4f4f4;
                margin: 0;
            }
            h1 {
                color: #333;
            }
            p {
                font-size: 18px;
                color: #666;
                text-align: center;
                margin: 20px 0;
            }
            a {
                text-decoration: none;
                padding: 10px 15px;
                background-color: #007bff;
                color: white;
                border-radius: 5px;
            }
            a:hover {
                background-color: #0056b3;
            }
        </style>
    </head>
    <body>
        <h1>)" + title + R"(</h1>
        <p>)" + content + R"(</p>
        <a href='/'>Return to homepage</a>
    </body>
    </html>
    )";
}
             
               
std::string generate_homepage() {
    return R"(
    <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Car Rental System</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            color: #333;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            padding: 20px;
        }

        h1, h2 {
            color: #444;
            text-align: center;
        }

        .form-container {
            display: flex;
            flex-direction: row;
            justify-content: space-around;
            width: 100%;
            max-width: 800px;
            margin-top: 20px;
        }

        form {
            display: flex;
            flex-direction: column;
            width: 100%;
            max-width: 400px;
            background-color: #fff;
            padding: 20px;
            border: 1px solid #ddd;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
        }

        input, select {
            padding: 12px;
            margin-bottom: 15px;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 16px;
            background-color: #f9f9f9;
            color: #333;
        }

        button, input[type="submit"] {
            padding: 12px;
            background-color: #28a745;
            color: #fff;
            font-size: 16px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
        }

        button:hover, input[type="submit"]:hover {
            background-color: #218838;
        }

        @media (max-width: 768px) {
            .form-container {
                flex-direction: column;
                align-items: center;
            }

            form {
                width: 90%;
                max-width: 100%;
            }
        }

    </style>
</head>
<body>
    <h1>CAR RENTAL SYSTEM</h1>

    <div class="form-container">
        <form action="/book_car" method="get">
            <h2>Book a Car</h2>
            <label for="userID">User ID:</label>
            <input type="text" id="userID" name="userID" placeholder="Enter your User ID" required>

            <label for="userName">User Name:</label>
            <input type="text" id="userName" name="userName" placeholder="Enter your name" required>

            <label for="choice">Car Category:</label>
            <select id="choice" name="choice">
                <option value="1">Luxury</option>
                <option value="2">Mid-Section</option>
                <option value="3">Low Category</option>
            </select>

            <label for="duration">Rental Duration (minutes):</label>
            <input type="number" id="duration" name="duration" placeholder="Enter duration in minutes" required>

            <input type="submit" value="Book Car">
        </form>

        <form action="/return_car" method="get">
            <h2>Return a Car</h2>
            <label for="userID">User ID:</label>
            <input type="text" id="userID" name="userID" placeholder="Enter your User ID" required>

            <input type="submit" value="Return Car">
        </form>
    </div>
</body>
</html>
    )";
}


int main() {
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    Aws::Client::ClientConfiguration config;
    config.region = "ap-south-1";
    Aws::DynamoDB::DynamoDBClient dynamoClient(config);

    RentalSystem system(dynamoClient);
    httplib::Server svr;

  
    svr.Get("/", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_content(generate_homepage(), "text/html");
    });

    
    svr.Get("/book_car", [&](const httplib::Request& req, httplib::Response& res) {
        std::string userID = req.get_param_value("userID");
        std::string userName = req.get_param_value("userName");
        int choice = std::stoi(req.get_param_value("choice"));
        int duration = std::stoi(req.get_param_value("duration"));

        std::string result = system.bookCar(userID, userName, choice, duration);
        res.set_content(generate_html("Booking of Car", result), "text/html");
    });


    svr.Get("/return_car", [&](const httplib::Request& req, httplib::Response& res) {
        std::string userID = req.get_param_value("userID");

        std::string result = system.returnCar(userID);
        res.set_content(generate_html("Return Car", result), "text/html");
    });

    std::cout << "Server started at http://localhost:8080" << std::endl;
    svr.listen("localhost", 8080);

    Aws::ShutdownAPI(options);
    return 0;
}
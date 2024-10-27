# Car Rental System

A simple car rental management system implemented in C++ that allows users to book and return cars with different rental durations and categories. The system integrates with AWS DynamoDB for storage and is accessible via a web-based interface.

## Features

- **Car Booking**: Users can book cars from three categories: Luxury, Mid-Section, and Low Category, each with its own per-minute pricing.
- **Car Return**: Users can return cars, and the system calculates the total rental cost, including a late fee if applicable.
- **AWS DynamoDB Integration**: Booking records are stored in a DynamoDB table.
- **Web Interface**: Provides a web-based interface to book and return cars using the [httplib](https://github.com/yhirose/cpp-httplib) library.

## Prerequisites

1. **AWS SDK for C++**: Installed and configured. See the [AWS SDK installation guide](https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/welcome.html).
2. **DynamoDB Table**: Ensure a DynamoDB table named `CAR_RENTAL` is set up in the `ap-south-1` region. The primary key should be `CAR_RENTAL` of type `S`.
3. **Dependencies**:
   - [httplib](https://github.com/yhirose/cpp-httplib): Lightweight HTTP server library.
   - AWS SDK for C++: Required for DynamoDB interaction.




#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <zaber/motion/ascii.h>

using namespace zaber::motion;
using namespace zaber::motion::ascii;

void find_absolute_beginnings(Axis& axis_x, Axis& axis_y, double& ABSOLUTE_BEGINNING_X, double& ABSOLUTE_BEGINNING_Y, const std::string& file_path);
void generate_and_save_coordinates(const std::string& file_path, double ABSOLUTE_BEGINNING_X, double ABSOLUTE_BEGINNING_Y, double x_steps, double y_steps, double x_step_size, double y_step_size, int stay_time_s);
void process_csv(const std::string& file_path, Axis& axis_x, Axis& axis_y);

std::string get_current_timestamp();

int main() {
    try {
        Library::checkVersion();

        Connection connection = Connection::openSerialPort("COM6");
        connection.enableAlerts();

        std::vector<Device> deviceList = connection.detectDevices();
        if (deviceList.size() < 2) {
            std::cerr << "At least two devices are required." << std::endl;
            return 1;
        }
        std::cout << "Found " << deviceList.size() << " devices" << std::endl;

        for (auto& device : deviceList) {
            std::cout << "Homing all axes of device with address " << device.getDeviceAddress() << "." << std::endl;
            device.getAllAxes().home();
        }

        std::cout << "WELCOME" << std::endl << std::endl;

        Device dev0 = deviceList[0];
        Device dev1 = deviceList[1];

        Axis axis_x = dev0.getAxis(1);
        Axis axis_y = dev1.getAxis(1);

        double ABSOLUTE_BEGINNING_X = 0;
        double ABSOLUTE_BEGINNING_Y = 0;

        double user_choice = 0;

        std::string file_path = "C:\\Users\\shres\\OneDrive\\Desktop\\data.csv";

        // Find absolute beginnings
        find_absolute_beginnings(axis_x, axis_y, ABSOLUTE_BEGINNING_X, ABSOLUTE_BEGINNING_Y, file_path);

        std::cout << "**************************************************" << std::endl;
        std::cout << "Choose from the options below:" << std::endl;
        std::cout << "  1) Write and read from a CSV." << std::endl;
        std::cout << "  2) Read from an existing CSV." << std::endl << std::endl;
        std::cout << "Enter your choice (1 or 2): " << std::endl;
        std::cin >> user_choice;
        std::cout << "**************************************************" << std::endl;

        if (user_choice == 1) {
            // Generate and save coordinates to CSV

            double x_steps = 0; // Number of steps in the x direction
            double y_steps = 0; // Number of steps in the y direction
            double x_step_size = 0; // Step size of x-direction movement
            double y_step_size = 0; // Step size of y-direction movement

            int stay_time_s = 0; // Stay time in seconds

            std::cout << "Enter the number of steps in x direction: ";
            std::cin >> x_steps;

            std::cout << "Enter the step size of steps in x direction (in mm): ";
            std::cin >> x_step_size;

            std::cout << "Enter the number of steps in y direction: ";
            std::cin >> y_steps;

            std::cout << "Enter the step size of steps in y direction (in mm): ";
            std::cin >> y_step_size;

            std::cout << "Enter the stay time for each movement (in seconds): ";
            std::cin >> stay_time_s;

            generate_and_save_coordinates(file_path, ABSOLUTE_BEGINNING_X, ABSOLUTE_BEGINNING_Y, x_steps, y_steps, x_step_size, y_step_size, stay_time_s);

            // Process the CSV for movements
            process_csv(file_path, axis_x, axis_y);

            std::cout << "***************************************************" << std::endl;
            std::cout << "COMPLETED" << std::endl;
            std::cout << "***************************************************" << std::endl;
        }
        else if (user_choice == 2) {
            std::cout << "NOTE: For windows, change \ to \\." << std::endl;
            std::cout << "Enter the file path of your csv file: " << std::endl;
            std::cin >> file_path;

            // Process the CSV for movements
            process_csv(file_path, axis_x, axis_y);

            std::cout << "***************************************************" << std::endl;
            std::cout << "COMPLETED" << std::endl;
            std::cout << "***************************************************" << std::endl;
        }
        else {
            std::cerr << "Invalid input. Please enter 1 or 2." << std::endl;
        }

        
    }
    catch (const std::exception& ex) {
        std::cerr << "An error occurred: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}

void find_absolute_beginnings(Axis& axis_x, Axis& axis_y, double& ABSOLUTE_BEGINNING_X, double& ABSOLUTE_BEGINNING_Y, const std::string& file_path) {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Could not open the file for writing: " << file_path << std::endl;
        return;
    }

    // Find ABSOLUTE_BEGINNING_X
    std::cout << "***************************************************" << std::endl;
    std::cout << "Accessing the x-axis stage." << std::endl;
    std::cout << "Keep entering the steps to find the Absolute Start." << std::endl;
    std::cout << "Enter 00 to stop the process and proceed." << std::endl;
    std::cout << "***************************************************" << std::endl;

    bool first_flag_x = true;
    while (first_flag_x) {
        double counter1 = 0;
        std::cout << "Enter steps you want to move (1 step = 1 mm distance) [00 to stop]: ";
        std::cin >> counter1;

        if (counter1 == 00) {
            first_flag_x = false;
        }
        else {
            axis_x.unpark();
            axis_x.moveRelative(counter1, Units::LENGTH_MILLIMETRES);
            axis_x.waitUntilIdle();
            axis_x.park();

            ABSOLUTE_BEGINNING_X += counter1;
            std::cout << "CHECK: Absolute Beginning value = " << ABSOLUTE_BEGINNING_X << std::endl;
        }
    }

    // Find ABSOLUTE_BEGINNING_Y
    std::cout << "***************************************************" << std::endl;
    std::cout << "Accessing the y-axis stage." << std::endl;
    std::cout << "Keep entering the steps to find the Absolute Start." << std::endl;
    std::cout << "Enter 00 to stop the process and proceed." << std::endl;
    std::cout << "***************************************************" << std::endl;

    bool first_flag_y = true;
    while (first_flag_y) {
        double counter1 = 0;
        std::cout << "Enter steps you want to move (1 step = 1 mm distance) [00 to stop]: ";
        std::cin >> counter1;

        if (counter1 == 00) {
            first_flag_y = false;
        }
        else {
            axis_y.unpark();
            axis_y.moveRelative(counter1, Units::LENGTH_MILLIMETRES);
            axis_y.waitUntilIdle();
            axis_y.park();

            ABSOLUTE_BEGINNING_Y += counter1;
            std::cout << "CHECK: Absolute Beginning value = " << ABSOLUTE_BEGINNING_Y << std::endl;
        }
    }

    // Save the absolute beginning positions to the CSV file
    file << ABSOLUTE_BEGINNING_X << "," << ABSOLUTE_BEGINNING_Y << ",0" << std::endl;
    file.close();

    std::cout << "Absolute beginning positions saved to " << file_path << std::endl;
}

void generate_and_save_coordinates(const std::string& file_path, double ABSOLUTE_BEGINNING_X, double ABSOLUTE_BEGINNING_Y, double x_steps, double y_steps, double x_step_size, double y_step_size, int stay_time_s) {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Could not open the file for writing: " << file_path << std::endl;
        return;
    }
    

    for (double y = 0; y < y_steps; ++y) {
        for (double x = 0; x < x_steps; ++x) {
            double x_position = ABSOLUTE_BEGINNING_X + (x * x_step_size);
            double y_position = ABSOLUTE_BEGINNING_Y + (y * y_step_size);
            file << x_position << "," << y_position << "," << stay_time_s << std::endl;
        }
    }

    file.close();
    std::cout << "Coordinates saved to " << file_path << std::endl;
}

std::string get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_time);
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}


void process_csv(const std::string& file_path, Axis& axis_x, Axis& axis_y) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << file_path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string x_str, y_str, stay_time_str, timestamp_str;

        try {
            if (std::getline(ss, x_str, ',') && std::getline(ss, y_str, ',') && std::getline(ss, stay_time_str, ',') && std::getline(ss, timestamp_str, ',')) {  // Add time stamp in the csv, only print it out. (Have hr:min:sec)
                double x_position = std::stod(x_str);
                double y_position = std::stod(y_str);
                int stay_time_s = std::stoi(stay_time_str);

                axis_x.unpark();
                axis_x.moveAbsolute(x_position, Units::LENGTH_MILLIMETRES);
                axis_x.waitUntilIdle();
                axis_x.park();

                axis_y.unpark();
                axis_y.moveAbsolute(y_position, Units::LENGTH_MILLIMETRES);
                axis_y.waitUntilIdle();
                axis_y.park();

                std::this_thread::sleep_for(std::chrono::seconds(stay_time_s));

                std::cout << "Moved to X: " << x_position << ", Y: " << y_position << " and waited for " << stay_time_s << " s." << std::endl;
            }
        }

        try {
            if (std::getline(ss, x_str, ',') && std::getline(ss, y_str, ',') && std::getline(ss, stay_time_str, ',') && std::getline(ss, timestamp_str, ',')) {
                double x_position = std::stod(x_str);
                double y_position = std::stod(y_str);
                int stay_time_s = std::stoi(stay_time_str);

                axis_x.unpark();
                axis_x.moveAbsolute(x_position, Units::LENGTH_MILLIMETRES);
                axis_x.waitUntilIdle();
                axis_x.park();

                axis_y.unpark();
                axis_y.moveAbsolute(y_position, Units::LENGTH_MILLIMETRES);
                axis_y.waitUntilIdle();
                axis_y.park();

                std::this_thread::sleep_for(std::chrono::seconds(stay_time_s));

                std::string timestamp = get_current_timestamp();
                output_file << x_position << "," << y_position << "," << stay_time_s << "," << timestamp << std::endl;

                std::cout << "Moved to X: " << x_position << ", Y: " << y_position << " and waited for " << stay_time_s << " s at " << timestamp << "." << std::endl;
            }
        }

        // Write the x,y, time stamp into a new csv (keep the same values from the read csv). Make sure to add the new time stamps everytime.


        catch (const std::exception& e) {
            std::cerr << "Error processing line: " << line << " - " << e.what() << std::endl;
        }
    }

    file.close();
}

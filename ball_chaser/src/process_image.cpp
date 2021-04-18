#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the drive_bot service
    if (!client.call(srv))
        ROS_ERROR("Failed to call service drive_bot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    int white_pixel = 255;

    // Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    int left_counter = 0;
    int foward_counter = 0;
    int right_counter = 0;

    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            int k = 3 * (i * img.width + j);
	    // get RGB pixel
            int r = img.data[k];
            int g = img.data[k+1];
            int b = img.data[k+2];
            if (r != white_pixel || g != white_pixel || b != white_pixel) {
                continue;
            }
            if (j < img.width / 3) {
                left_counter++;
            } else if (j > 2 * img.width / 3) {
                right_counter++;
            } else {
                foward_counter++;
            }
        }
    }

    if (left_counter == 0 && foward_counter == 0 && right_counter == 0) {
        drive_robot(0.0, 0.0); // stop
    } else if (left_counter > foward_counter && left_counter > right_counter) {
        drive_robot(0.0, 0.3); // move left
    } else if (right_counter > foward_counter) {
        drive_robot(0.0, -0.3); // move right
    } else {
        drive_robot(0.3, 0.0); // move foward
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}


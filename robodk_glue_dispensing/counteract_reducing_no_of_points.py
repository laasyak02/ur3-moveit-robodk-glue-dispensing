from robodk.robodialogs import *
from robodk.robofileio import *
from robodk.robolink import *

import cv2
import numpy as np
import pyrealsense2 as rs
import csv
import time
from scipy.interpolate import interp1d

RDK = Robolink()

robot = RDK.ItemUserPick('Select a robot', ITEM_TYPE_ROBOT)
if not robot.Valid():
    raise Exception("Robot not selected or not valid")
    quit()

# Get the reference frame to generate the path
#frame = RDK.Item(REFERENCE_NAME,ITEM_TYPE_FRAME)
frame = robot.getLink(ITEM_TYPE_FRAME)
if not frame.Valid():
    frame = robot.Parent()

if not frame.Valid():
    raise Exception("Reference frame not found")

# Set the robot to the home position
# robot.setJoints(target.Joints())
'''
if robot.Connect() > 0:
    joints = robot.Joints()
else:
    raise Exception("Unable to connect to robot")
'''
# Get the pose reference from the home target
joints1 = [[-124.119], [-129.241], [-109.691], [-31.503], [90.930], [-34.034]]
#print(type(joints))
#print(type(robomath.Mat(joints1)))
joints1 = robomath.Mat(joints1)
robot.MoveJ(joints1)
time.sleep(2)


# Initialize the RealSense pipeline
pipeline = rs.pipeline()
config = rs.config()
config.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)
config.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 30)

# Start streaming
pipeline.start(config)

try:
    while True:
        time.sleep(5)
        # Wait for a new frame
        frames = pipeline.wait_for_frames()
        color_frame = frames.get_color_frame()
        depth_frame = frames.get_depth_frame()
        if not color_frame or not depth_frame:
            continue

        # Convert the RealSense frame to OpenCV images
        color_image = np.asanyarray(color_frame.get_data())
        depth_image = np.asanyarray(depth_frame.get_data())

        # Convert depth image to point cloud
        points = rs.pointcloud()
        points.map_to(color_frame)
        point_cloud = points.calculate(depth_frame)

        # Convert point cloud to numpy array
        vertices = np.asanyarray(point_cloud.get_vertices()).view(np.float32).reshape(-1, 3)  # 3D coordinates

        # Convert to grayscale
        gray_image = cv2.cvtColor(color_image, cv2.COLOR_BGR2GRAY)

        # Apply thresholding (adjust parameters as needed)
        _, binary_image = cv2.threshold(gray_image, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)

        # Find contours
        contours, _ = cv2.findContours(binary_image, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        # Filter external contours based on area
        external_contours = []
        max_area = 0
        for contour in contours:
            area = cv2.contourArea(contour)
            if area > max_area:
                max_area = area
                external_contours = [contour]

        # Draw external contours on the color image
        output_image = color_image.copy()
        cv2.drawContours(output_image, external_contours, -1, (0, 255, 0), 2)

        # Get real-world coordinates (X, Y, Z) for contour points
        if external_contours:
            contour = external_contours[0]
            contour_points_2d = contour.reshape(-1, 2)
            contour_points_3d = []
            for point_2d in contour_points_2d[:1000]:  # Limit to first 100 points
                depth_value_m = depth_image[int(point_2d[1]), int(point_2d[0])] * pipeline.get_active_profile().get_device().first_depth_sensor().get_depth_scale()
                if depth_value_m == 0:  # Invalid depth value
                    continue
                depth_point = vertices[int(point_2d[1]) * 640 + int(point_2d[0])] * 1000  # Convert meters to millimeters
                # Apply 30 percent counteraction
                depth_point *= 0.6
                depth_point = np.append(depth_point, [100])  # Add a constant speed value
                contour_points_3d.append(depth_point)

            #print(contour_points_3d)
            #print(type(contour_points_3d))
            #print(len(contour_points_3d))
            #print(type(contour_points_3d[0][2]))

            # Calculating average of z values
            z_val=[]

            for i in range(len(contour_points_3d)):
                z_val.append(contour_points_3d[i][2])

            #print(len(z_val))
            z_avg = np.mean(z_val)
            #print(z_avg)
            # Taking highest point on the object which is least z value
            z_val_min = np.min(z_val)

            # Assigning average of z values as the z value for each point
            for i in range(len(contour_points_3d)):
                contour_points_3d[i][2] = z_val_min

            '''
            points=contour_points_3d
            #window_size=5
            #filtered_points = []
            #half_window = window_size // 2
            num_points=50

            x_nu = [point[0] for point in points]
            y_nu = [point[1] for point in points]

            unique_indices = np.unique(x_nu, return_index=True)[1]
            x = x_nu[unique_indices]
            y = y_nu[unique_indices]

            # Perform spline interpolation
            spline_func = interp1d(x, y, kind='cubic')

            # Generate new x values for interpolation
            new_x = np.linspace(min(x), max(x), num_points)

            # Interpolate y values corresponding to the new x values
            new_y = spline_func(new_x)

            # Combine interpolated x and y coordinates
            interpolated_points = [(x_val, y_val) for x_val, y_val in zip(new_x, new_y)]

            for i, point in enumerate(contour_points_3d[:1000]):
                point[0] = interpolated_points[i][0]
                point[1] = interpolated_points[i][1]
            '''
            contour_length = len(contour_points_3d)

            # Define a factor to determine the group size based on the contour length
            group_factor = 0.02  # You can adjust this factor based on your specific needs

            # Calculate the group size based on the contour length
            group_size = max(3, int(contour_length * group_factor))

            contour_points_3d_shortened=[]
            xy=[]
            for i, point in enumerate(contour_points_3d[:1000]):
                xy.append((point[0],point[1]))

            for i in range(0, len(xy), group_size):  # Group points in threes
                group_points = xy[i:i + group_size]
                median_point = np.median(group_points, axis=0)
                contour_points_3d_shortened.append((median_point[0],median_point[1],z_val_min,100))

            #print(contour_points_3d)
            #print(type(contour_points_3d))
            #print(len(contour_points_3d))
            print(len(contour_points_3d_shortened))
            print(len(contour_points_3d))

            # Adding a point to go back to the original position
            #last_point1 = contour_points_3d[len(contour_points_3d)-1]
            #last_point = last_point1
            #last_point[0] = last_point1[0]*(np.float64(-1.0))
            #last_point[1] = last_point1[1]*(np.float64(-1.0))
            #last_point[2] = last_point1[2]*(np.float64(-1.0))
            #contour_points_3d.append(last_point)

            # file_name = 'C:\Users\SDC-4\Desktop\contour real\contour_coordinates_square2.csv'

            # Save contour coordinates to CSV
            with open('contour_coordinates_square2_med_filt.csv', 'w', newline='') as csvfile:
                writer = csv.writer(csvfile)
                writer.writerow(['X', 'Y', 'Z', 'Speed'])
                writer.writerows(contour_points_3d_shortened)

        # Display the result
        cv2.imshow('External Contours', output_image)
        cv2.waitKey(1)
        time.sleep(5)

        break

        # Check for exit key
        # if cv2.waitKey(1) & 0xFF == ord('q'):
            # break

finally:
    # Stop streaming
    pipeline.stop()

    # Close OpenCV windows
    cv2.destroyAllWindows()

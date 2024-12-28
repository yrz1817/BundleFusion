'''
在按下c的时候相机尽可能处于移动状态
在达到4000张的时候自动退出，或按q手动退出
相机尽量匀速顺滑移动
'''
import pyrealsense2 as rs
import numpy as np
import cv2
import time
import os

# 创建 RealSense 管道
pipeline = rs.pipeline()
config = rs.config()

# 启用深度流和颜色流
config.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)
config.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 30)

# 创建对齐对象，设置为对齐颜色流
align = rs.align(rs.stream.color)

# 设置保存文件夹路径
output_dir = r"D:\BundleFusion2\sens\output"
os.makedirs(output_dir, exist_ok=True)

# 启动管道
pipeline.start(config)

# 等待一段时间以让相机稳定
time.sleep(2)

try:
    print("Press 'c' to start capturing images, 'q' to quit.")

    capturing = False  # 控制捕获状态
    frame_count = 0    # 帧计数器

    while True:
        frames = pipeline.wait_for_frames()
        aligned_frames = align.process(frames)  # 对齐帧
        depth_frame = aligned_frames.get_depth_frame()
        color_frame = aligned_frames.get_color_frame()

        if not depth_frame or not color_frame:
            continue

        # 将图像转换为 numpy 数组
        depth_image = np.asanyarray(depth_frame.get_data())
        color_image = np.asanyarray(color_frame.get_data())

        # 显示图像
        cv2.imshow('Color Image', color_image)
        cv2.imshow('Depth Image', depth_image)

        # 按 'c' 键开始捕获，按 'q' 键退出
        key = cv2.waitKey(1) & 0xFF
        if key == ord('c'):
            capturing = True
            print("Capturing started...")
            frame_count = len(os.listdir(output_dir))  # 初始化帧计数

        elif key == ord('q'):
            break

        # 捕获图像
        if capturing:
            depth_filename = os.path.join(output_dir, f"frame-{frame_count:06d}.depth.png")
            color_filename = os.path.join(output_dir, f"frame-{frame_count:06d}.color.jpg")
            pose_filename = os.path.join(output_dir, f"frame-{frame_count:06d}.pose.txt")

            cv2.imwrite(depth_filename, depth_image)
            cv2.imwrite(color_filename, color_image)
            with open(pose_filename, 'w') as file:
                file.write("1 0 0 0\n")
                file.write("0 1 0 0\n")
                file.write("0 0 1 0\n")
                file.write("0 0 0 1\n")
            print(f"Captured frame {frame_count}")

            frame_count += 1  # 增加帧计数

            # 自动退出条件
            if frame_count >= 4000:
                print("Captured 4000 frames, exiting...")
                break

finally:
    # 停止管道
    pipeline.stop()
    cv2.destroyAllWindows()
    print(f"Data saved in {output_dir}")

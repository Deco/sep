
class ActuatorController {
    /* Parameters:
        dynamixel_servo_yaw_id          int
        dynamixel_servo_yaw_rawmin      int
        dynamixel_servo_yaw_rawmax      int
        dynamixel_servo_pitch_id        int
        dynamixel_servo_pitch_rawmin    int
        dynamixel_servo_pitch_rawmax    int
    */
    ActuatorController(const std::string device_name)
    void update();
    void getCurrentPosition(cv::Vec2d &posRef);
    void move(const cv::Vec2d goalPos, const double time);
    void stop();
    void getPositionRange(cv::Vec2d &min, cv::Vec2d &max);
};

class ThermopileSensor {
    
}
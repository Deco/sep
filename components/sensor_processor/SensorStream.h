
// A sample of several "pixels"
class SensorSample
{
public:
    SensorSampleBase();
    ~SensorSampleBase();
};

// A continuous stream of samples
class SensorStream
    <typename SampleT = SensorSampleBase>
{
public:
    SensorStreamBase();
    ~SensorStreamBase();

};

class MLX90620ThermopileSample
    : SensorSample
{
public:
    cv::Mat_<float> data;
public:
    MLX90620ThermopileSample();
    ~MLX90620ThermopileSample();
};

class MLX90620ThermopileStream
    : SensorSample<MLX90620ThermopileSample>
{
public:
    MLX90620Thermopile();
    ~MLX90620Thermopile();
    
};

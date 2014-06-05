#include "SensorSamplerRealm.h"

int main()
{
    return 1;
}

SensorSamplerRealm::SensorSamplerRealm(double inMaxTilt, double inMaxPan, 
														double inReadingSize)
{
	maxPan = inMaxPan;
	maxTilt = inMaxTilt;
	readingSize = inReadingSize;
}

void SensorSamplerRealm::addReadings(cv::Mat_<float> &matRef, cv::Vec2d pos, 
																double time)
{
	for (int i = 0; i < matRef.rows; i++)
	{
		for (int j = 0; j < matRef.cols; j++)
		{
			ReadingInfo r;
			r.position[0] = pos[0] - (readingSize*(((double)matRef.cols)/2.0)) + 
										(j*readingSize) + (readingSize/2.0);
			r.position[1] = pos[1] - (readingSize*(((double)matRef.rows)/2.0)) + 
										(i*readingSize) + (readingSize/2.0);
			r.tempValue = matRef(i,j);
			r.creationTime = time;
			list.emplace_back(r);
		}
	}
}

void SensorSamplerRealm::updateWindow(SensorSampleWindow &windowRef)
{
	/// iter through our window pos for the pixels.
	for (int i = 0; i < windowRef.img.rows; i++)
	{
		for (int j = 0; j < windowRef.img.cols; j++)
		{
			cv::Rect cell(j*readingSize, i*readingSize, readingSize, readingSize);
			double sum = 0;
			int num = 0;
			for (ReadingInfo r : list)
			{
				cv::Rect reading(r.position[0], r.position[1], readingSize, readingSize);
                if((cell & reading).area() != 0) 
				{
					sum += r.tempValue;
					num++;
				}
			}
    		sum = (num == 0 ? 0 : sum/num);
            windowRef.img(i,j) = sum;
		}
	}
}
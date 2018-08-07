#ifndef img_hpp
#define img_hpp

#include "opencv2/opencv.hpp"

class Img_put{
  public:
    Img_put();
    virtual ~Img_put();
    void Img_put(class Img_put)=delete;
    void Img_put(class Img_put)&&=delete;
    void operator=()=default;
    void draw_img_on_img();
    cv::Mat show_img_src()const;
  private:
    cv::Mat img_src;
    void draw_img();
}
#endif

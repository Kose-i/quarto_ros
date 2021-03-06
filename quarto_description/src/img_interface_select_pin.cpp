#include <iostream>
#include <bitset>
#include <thread>
#include <chrono>
#include <string>

#include <ros/ros.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "quarto/bridge.h"

const std::string path_str{"/home/tamura-kosei/works/board_game_ros/src/quarto/img/"};
constexpr int target_size{9};

int global_pin = 10;//parfect not exist
int before_pin = global_pin;

int width {};
int height {};
std::vector<struct pos> vec(target_size);
std::bitset<target_size> isexist;
namespace window_name{
  const std::string board_name{"board_img"};
  const std::string pin_img{"pin_img"};
}
void initialize_isexist() {
  for (int i {};i < target_size;++i) {
    isexist.set(i);
  }
}

struct pos{
  int x;
  int y;
  int width;
  int height;
};


void callback_mouse(int event, int x, int y, int flags, void*)
{
  switch(event){
    case CV_EVENT_LBUTTONDOWN:
    case CV_EVENT_RBUTTONDOWN:
      global_pin = (y / ((height + 3)/3))*3 + x / (width/ 3);
      break;
  }
}

// 画像を画像に貼り付ける関数
void paste_mat_img(cv::Mat src, cv::Mat dst, const int& x, const int& y, const int& resize_width, const int& resize_height) {
  ROS_INFO("%d %d %d %d", x,y,resize_width, resize_height);

	cv::Mat resized_img;
	cv::resize(src, resized_img, cv::Size(resize_width, resize_height));

	if (x >= dst.cols || y >= dst.rows) return;
	int w = (x >= 0) ? std::min(dst.cols - x, resized_img.cols) : std::min(std::max(resized_img.cols + x, 0), dst.cols);
	int h = (y >= 0) ? std::min(dst.rows - y, resized_img.rows) : std::min(std::max(resized_img.rows + y, 0), dst.rows);
	int u = (x >= 0) ? 0 : std::min(-x, resized_img.cols - 1);
	int v = (y >= 0) ? 0 : std::min(-y, resized_img.rows - 1);
	int px = std::max(x, 0);
	int py = std::max(y, 0);
  ROS_INFO("%d %d %d %d px:%d py:%d", w, h, u, v, px, py);

	cv::Mat roi_dst = dst(cv::Rect(px, py, w, h));
	cv::Mat roi_resized = resized_img(cv::Rect(u, v, w, h));
	roi_resized.copyTo(roi_dst);
}

void paste_mat_img(cv::Mat src, cv::Mat dst, const struct pos& select) {
  paste_mat_img(src, dst, select.x, select.y, select.width ,select.height);
}

const std::string pin_box[] = {"first", "second", "third", "fourth","fifth","sixth","seventh","eighth","ninth"};

int main(int argc, char** argv){
  ros::init(argc, argv, "quarto_select_pin_client");
  ros::NodeHandle nh;

  cv::Mat pin_img_src = cv::imread(path_str+"pin_img.png", cv::IMREAD_COLOR);
  if(pin_img_src.empty()){
    ROS_INFO("can't open temp_picture");
    return -1;
  }

  cv::Mat color_pin_img_src = cv::imread(path_str+"coloring_pin_img.png",cv::IMREAD_COLOR);
  if (color_pin_img_src.empty()) {
    ROS_INFO("can't open highlight_img");
    return -1;
  }

  cv::Mat image_blank = cv::imread(path_str+"blank_img.png", cv::IMREAD_COLOR);
  if (image_blank.empty()) {
    ROS_ERROR("not open blank_img");
    return -1;
  }

  ROS_INFO("START %s","select_pin" );

  width = pin_img_src.size().width;
  height = pin_img_src.size().height;
  ROS_INFO("width: %d height: %d", width, height);

  for (int i {};i < 3;++i) {
    for (int j {}; j < 3;++j) {
      vec[i*3 + j].x = (width* j) / 3;
      vec[i*3 + j].y = (height*i) / 3;
      vec[i*3 + j].width =  width/3;
      vec[i*3 + j].height = height / 3;
    }
  }


  quarto::bridge srv;
  srv.request.str_pin = '0';// = pin_box[global_pin];
  ros::ServiceClient client = nh.serviceClient<quarto::bridge>("select_pin");
  ROS_INFO("Start client");
  initialize_isexist();

  cv::namedWindow(window_name::pin_img);
  cv::setMouseCallback(window_name::pin_img, &callback_mouse);

  while(cv::waitKey(1) != 'q'){
    cv::imshow(window_name::pin_img, pin_img_src);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    if(global_pin != before_pin){
      if(0 <= global_pin && global_pin < target_size && isexist[global_pin] == true){
        srv.request.str_pin = pin_box[global_pin];
        client.call(srv);
        ROS_INFO("THROW CLIENT");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if(srv.response.str_answer == "ng") {
          continue;
        } else if (srv.response.str_answer == "ok") {
          paste_mat_img(image_blank, pin_img_src, vec[global_pin]);
          ROS_INFO("global pin:%d", global_pin + 1);
          isexist.reset(global_pin);
          before_pin = global_pin;
        }
      }
    }

  }
  ros::spin();
  return 0;
}

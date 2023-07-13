#pragma once


class Timer {
public:
  Timer (int interval = 3) {
    _interval = interval;
  }
  
  ~Timer () = default;

  void start () {
    _left = millis ();
  }
  
  void stop () {
    EVERY_N_SECONDS (_interval) {
      _right = millis ();
      Serial.print("frame duration: ");
      Serial.println(_right - _left);
      Serial.print("fps: ");
      Serial.println(1000 / (_right - _left));
    }
  }

  
private:
  int _interval;
  unsigned long int _left = 0;
  unsigned long int _right = 0;
};

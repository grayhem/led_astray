#pragma once



template <
    EOrder RGB_ORDER = RGB,
    uint8_t CHIP = WS2811_800kHz
>
class TeensyController : public CPixelLEDController<RGB_ORDER, 8, 0xFF>
{
    OctoWS2811*   _pocto ;
public:
    TeensyController (OctoWS2811* pocto): _pocto (pocto) {};

    virtual void init() {};
    virtual void showPixels (PixelController<RGB_ORDER, 8, 0xFF> &pixels)
    {

        uint32_t i = 0;
        while (pixels.has (1))
        {
            uint8_t r = pixels.loadAndScale0 ();
            uint8_t g = pixels.loadAndScale1 ();
            uint8_t b = pixels.loadAndScale2 ();
            _pocto->setPixel (i, r, g, b);
            pixels.stepDithering ();
            pixels.advanceData ();
            ++i;
        }

        _pocto->show ();
    }
};

template<int len>
class Smoother 
{
public:
  Smoother () 
  {
    for (int i = 0; i < len; ++i) {
      _buffer[i] = 0;
    }
  }

  ~Smoother () = default;

  int update (const int& value)
  {
    _idx = (_idx + 1) % len;
    _buffer [_idx] = value;
    int ret = 0;
    for (size_t i = 0; i < len; ++i) {
      ret += _buffer[i];
    }
    ret /= len;
    return ret;
  }

private:
  int   _buffer[len];
  int   _idx = 0;
};

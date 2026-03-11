#include "LSM1x0A_Controller.h"
#include <Arduino.h>
#include <NeoPixelAnimator.h>
#include <NeoPixelBus.h>

const uint16_t PixelCount   = 10;   // make sure to set this to the number of pixels in your strip
const uint16_t PixelPin     = 4;    // make sure to set this to the correct pin, ignored for Esp8266
const uint16_t AnimCount    = 1;    // we only need one
const uint16_t TailLength   = 5;    // length of the tail, must be shorter than PixelCount
const float    MaxLightness = 0.4f; // max lightness at the head of the tail (0.5f is full bright)

NeoGamma<NeoGammaTableMethod>                colorGamma; // for any fade animations, best to correct gamma
NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip(PixelCount, PixelPin);
NeoPixelAnimator                             animations(AnimCount);

void verboseEventCallback(LsmLogLevel level, const char* component, const char* message)
{
  const char* levelStr = "UNK";
  switch (level) {
    case LsmLogLevel::ERROR:
      levelStr = "ERR";
      break;
    case LsmLogLevel::WARN:
      levelStr = "WRN";
      break;
    case LsmLogLevel::INFO:
      levelStr = "INF";
      break;
    case LsmLogLevel::DEBUG:
      levelStr = "DBG";
      break;
    case LsmLogLevel::VERBOSE:
      levelStr = "VRB";
      break;
    default:
      break;
  }
  Serial.printf("[%s][%s] %s\n", levelStr, component, message);
}

void LoopAnimUpdate(const AnimationParam& param)
{
  // wait for this animation to complete,
  // we are using it as a timer of sorts
  if (param.state == AnimationState_Completed) {
    // done, time to restart this position tracking animation/timer
    animations.RestartAnimation(param.index);

    // rotate the complete strip one pixel to the right on every update
    strip.RotateRight(1);
  }
}

void DrawTailPixels(const RgbColor& baseColor)
{
  // Transición de brillo usando el color base
  for (uint16_t index = 0; index < strip.PixelCount() && index <= TailLength; index++) {
    float lightness = index * MaxLightness / TailLength;
    // Escalar el color base según lightness
    RgbColor color(uint8_t(baseColor.R * lightness), uint8_t(baseColor.G * lightness), uint8_t(baseColor.B * lightness));
    strip.SetPixelColor(index, colorGamma.Correct(color));
  }
}

void setupLed(bool success)
{
  strip.Begin();
  strip.Show();

  // Color base según estado
  RgbColor baseColor = success ? RgbColor(0, 255, 0) : RgbColor(255, 0, 0);
  strip.ClearTo(baseColor);

  DrawTailPixels(baseColor);
  animations.StartAnimation(0, 100, LoopAnimUpdate);
}

void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n--- Test: RX, TX, RESET PIN ---");
  LSM1x0A_Controller* controller = new LSM1x0A_Controller();
  controller->setLogCallback(verboseEventCallback, LsmLogLevel::VERBOSE);
  controller->begin();

  for (int i = 0; i < 5; i++) {
    if (controller->wakeUp()) {
      Serial.println("[APP] WakeUp OK.");
      break;
    }
    else {
      Serial.println("[APP] WakeUp falló.");

      setupLed(false);
      return;
    }
    delay(1000);
  }

  // Test software reset
  Serial.println("\n[APP] Probando Software Reset...");
  if (controller->softwareReset()) {
    Serial.println("[APP] Software Reset OK.");
  }
  else {
    Serial.println("[APP] Software Reset falló.");
    setupLed(false);
    return;
  }

  // Test hardware reset
  Serial.printf("\n[APP] Probando Hardware Reset en GPIO %d...\n", 15);
  if (controller->hardwareReset()) {
    Serial.println("[APP] Hardware Reset OK.");
  }
  else {
    Serial.println("[APP] Hardware Reset falló.");
    setupLed(false);
    return;
  }

  Serial.println("\n[APP] Todos los tests pasaron correctamente.");
  setupLed(true);
}

void loop()
{
  animations.UpdateAnimations();
  strip.Show();
}
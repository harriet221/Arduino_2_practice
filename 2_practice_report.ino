#include <WiFi.h> // 와이파이 기본(STATION) 모드
#include <WiFiClient.h> // AP 모드 - (공유기 접근 위한) 클라이언트 접속
#include <WiFiAP.h> // AP 모드(유무선 공유기)

// 헤더파일 중복 방지
#ifndef STASSID
#define STASSID "KT_GiGA_2G_4BC4" // SSID(와이파이 이름) 정의
#define STAPSK "7ef72xc327" // 비밀번호 정의
#endif

const char* ssid = STASSID; // 와이파이 이름
const char* password = STAPSK; // 비밀번호
WiFiServer server(80); // 80(HTTP) 서버 지정

int vResistor = A6; // ADC 6 (GPIO 34)
int led_a = 18; // PWM_LED GPIO 18
int led_b = 19; // LED GPIO 19

const int pwm_channel = 0; // 0~15의 PWM 채널 중 하나 지정
boolean h_pwm = false; // 꺼짐
boolean pwm1 = true; // 켜짐

void setup() {
  Serial.begin(115200); // 시리얼 모니터(PC) 데이터 전송 속도 115200
  pinMode(vResistor, INPUT); // 가변저항 입력 설정
  pinMode(led_a, OUTPUT); // 18번 pwm_led 출력 설정
  pinMode(led_b, OUTPUT); // 19번 led 출력 설정
  digitalWrite(led_b, 0); // 19번 led 끈 걸로 시작
  ledcAttachPin(led_a, pwm_channel); // 18번 pwm_led PWM 채널 지정
  ledcSetup(pwm_channel, 5000, 8); // PWM 채널, 주파수, 해상도 설정
  ledcWrite(pwm_channel, h_pwm); // 18번 pwm_led 끈 걸로 시작

  // 와이파이 연결 확인
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid); // 와이파이 이름 출력

  WiFi.mode(WIFI_STA); // STATION 모드
  WiFi.begin(ssid, password); // 와이파이 연결

  // 아직 연결 중일 때 (....) 출력
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // 와이파이 연결 확인 및 IP 주소 출력 (복사하여 주소창에 붙여넣으면 됨)
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin(); // 서버 연결
}

void loop() {
  WiFiClient client = server.available(); // 클라이언트 접속으로 서버값 읽어오기
  
  if(!client) return; // 서버값 없으면 재접속
  Serial.println("새로운 클라이언트");
  client.setTimeout(5000);

  String request = client.readStringUntil('\r');
  Serial.println("request: ");
  Serial.println(request);

  int val_a; // 18번 led 값
  int val_b; // 19번 led 값

  // 주소뒤에 붙는 조건에 따라 19번 led 끄고 켬
  if(request.indexOf(F("/gpio/0")) != -1){
    val_b = 0;
  } else if (request.indexOf(F("/gpio/1")) != -1){
    val_b = 1;
  } else {
    Serial.println("invalid request");
    val_b = digitalRead(led_b);
  }

  digitalWrite(led_b, val_b); // 

  int adc = analogRead(vResistor); // 가변저항 값 아날로그로 읽어오기
  int pwm = map(adc, 0, 1023, 0, 255); // 해당 아날로그 신호 디지털 표현 위해 값 매핑

  ledcWrite(pwm_channel, pwm); // analogWrite() 대체함수 - 해당 채널 led 해당 값에 따라 출력
  
  while(client.available()){ // 클라이언트(서버) 신호 있으면 읽어들이기
    client.read();
  }

  // html - 웹 페이지 출력 (19번 LED 켜고 꺼짐 여부 / 19번 LED 스위칭 문장 / 가변저항 값)
  client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now "));
  client.print((val_b) ? F("high") : F("low"));
  client.print(F("<br><br>Click <a href='http://"));
  client.print(WiFi.localIP());
  client.print(F("/gpio/1'>here</a> to switch LED GPIO on, or <a href='http://"));
  client.print(WiFi.localIP());
  client.print(F("/gpio/0'>here</a> to switch LED GPIO off."));
  client.print(F("<br><br>vResistor is now "));
  client.print(adc);
  client.print(F(".</html>"));

  Serial.println("Disconnecting from client"); // (출력 끝남 = 다 했으니 클라이언트 연결 다시 하시오)

}

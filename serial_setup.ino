// for USBCON
//#include <USBAPI.h>

void serial_setup(unsigned long baudrate) {
  /*
    while(!Serial) will hang until serial-console is opened.

    !Serial -- opened on host, using usblineinfo.linestate > 0, so dtr or rts or...?
      about 10msec after UDFUML/_usbConfiguration
    .dtr -- effectively the same, actually linestate bit 0x1
    #define USBCON used for adding .dtr() and other extra functions to serial
      ends up controlling whether SerialUSB gets defined
    #define HAVE_CDCSERIAL

    USBDevice.configured() 0 for no
      .FrameNumber() that incremented count? yes... but:
      .USBConnected() are we connected, has delay(3)!
        declare: uint8_t USBConnected();
        will not be >0 until the host usb connects (about 200msec)

      at attach():
      _usbConfiguration 0, !=0 if cable gets connected

  */
  // we show progress w/flashing LED
  pinMode(LED_BUILTIN, OUTPUT);

  unsigned long start = millis();

  boolean try_serial = false;
  unsigned long timeout = 0;
  
#ifdef SerialUSB
  // wait for usb "transport" up to timeout
  // we'll use UDFML>0 as usb connected (not Serial connected)
  // could also use _usbConfiguration != 0
  timeout = 500; // by experiment depends on host usb driver's response time, and then the app's open time
  while ( millis() - start < timeout && !UDFNUML) {
    delay(20);
    digitalWrite(LED_BUILTIN, ! digitalRead(LED_BUILTIN));
  }
  if (UDFNUML) {
    try_serial = true;

    // time for app to open serial, we are assuming something like Arduino IDE that reacts pretty quick
    // mine reacted in 40msec
    timeout += 1500;
  }
#else
  try_serial = true;
  timeout += 0; // non-usb means uart, which is always present
#endif
  unsigned long connect_finished = millis();

  if (try_serial) {
    // usb-cdc `Serial` will be false till host "opens" the serial,
    // which can hang us if no usb host, or no host app opens serial
    // uart `Serial` will always be true
    unsigned long open_start = millis();
    Serial.begin(baudrate);

    // wait for serial, up to timeout
    while (millis() - open_start < timeout && !Serial) {
      delay(40);
      digitalWrite(LED_BUILTIN, ! digitalRead(LED_BUILTIN));
    }

    unsigned long finished = millis();
    //while (!Serial) {
    //  delay(10);
    //}
#ifdef SerialUSB
    Serial.print(F("SerialUSB "));
    Serial.println( try_serial ? "connected " : "not connected");
#else
    Serial.println("UART Serial");
#endif
    //Serial << F("before startup took ") << start << endl;
    if (finished - connect_finished) {
      Serial.print("connect took "); Serial.println( connect_finished - start);
    }
    if (finished - start > 1) {
      Serial.print("Serial delay total "); Serial.println( finished - start);
    }
  }

#ifdef SerialUSB
  // stall a total of 3secs to give a human time to upload before main code
  // in cases where main code has a bad hang/crash in it (and uplaod doesn't work)
  // uart Serial has DTR hooked to cpu reset, so always works on upload
  // usb-cdc requires the usb code to run on the arduino to detect reset for upload
  // and a bad hang/crash can stop usb from working.

  if (UDFNUML) { // but, don't bother unless we have a usb connected
    unsigned long upload_timeout = 3000; // - (millis() - start);
    if ( millis() - start < upload_timeout ) {
      //Serial << F("upload timeout ") << (upload_timeout - (millis() - start) ) << endl;
      while (millis() - start < upload_timeout) {
        delay(80);
        digitalWrite(LED_BUILTIN, ! digitalRead(LED_BUILTIN));
      }
    }
  }
#endif
  //if (Serial) Serial << F("Total serial setup time ") << (millis() - start) << endl;

  ///// debug
  //delay(2000);
  //Serial.println( try_serial ? "connected " : "not connected");
  //Serial.println();


}

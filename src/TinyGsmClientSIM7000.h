/**
 * @file       TinyGsmClientSIM7000.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTSIM7000_H_
#define SRC_TINYGSMCLIENTSIM7000_H_

// #define TINY_GSM_DEBUG Serial
// #define TINY_GSM_USE_HEX

#define TINY_GSM_MUX_COUNT 2
#define TINY_GSM_BUFFER_READ_AND_CHECK_SIZE

#include "TinyGsmClientSIM70xx.h"
#include "TinyGsmTCP.tpp"


class TinyGsmSim7000 : public TinyGsmSim70xx<TinyGsmSim7000>,
                       public TinyGsmTCP<TinyGsmSim7000, TINY_GSM_MUX_COUNT> {
  friend class TinyGsmSim70xx<TinyGsmSim7000>;
  friend class TinyGsmTCP<TinyGsmSim7000, TINY_GSM_MUX_COUNT>;

  /*
   * Inner Client
   */
 public:
  class GsmClientSim7000 : public GsmClient {
    friend class TinyGsmSim7000;

   public:
    GsmClientSim7000() {}

    explicit GsmClientSim7000(TinyGsmSim7000& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmSim7000* modem, uint8_t mux = 0) {
      this->at       = modem;
      sock_available = 0;
      prev_check     = 0;
      sock_connected = false;
      got_data       = false;

      if (mux < TINY_GSM_MUX_COUNT) {
        this->mux = mux;
      } else {
        this->mux = (mux % TINY_GSM_MUX_COUNT);
      }
      at->sockets[this->mux] = this;

      return true;
    }

   public:
    virtual int connect(const char* host, uint16_t port, int timeout_s) {
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    void stop(uint32_t maxWaitMs) {
      dumpModemBuffer(maxWaitMs);
      at->sendAT(GF("+CACLOSE="), mux);
      sock_connected = false;
      at->waitResponse(3000);
    }
    void stop() override {
      stop(15000L);
    }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  };

  /*
   * Inner Secure Client
   */
<<<<<<< HEAD

  class GsmClientSecureSIM7000 : public GsmClientSim7000 {
   public:
    GsmClientSecureSIM7000() {}

    GsmClientSecureSIM7000(TinyGsmSim7000& modem, uint8_t mux = 0)
        : GsmClientSim7000(modem, mux) {}

   public:
    int connect(const char* host, uint16_t port, int timeout_s) override {
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      sock_connected = at->modemConnect(host, port, mux, true, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES
  };
=======
  // NOTE:  Use modem TINYGSMSIM7000SSL for a secure client!
>>>>>>> 6e10a3a009d121b0b5ad2f04b9ecbea379d10170

  /*
   * Constructor
   */
 public:
  explicit TinyGsmSim7000(Stream& stream)
      : TinyGsmSim70xx<TinyGsmSim7000>(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSIM7000"));

    if (!testAT()) { return false; }

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    DBG(GF("### Modem:"), getModemName());

    // Enable Local Time Stamp for getting network time
    sendAT(GF("+CLTS=1"));
    if (waitResponse(10000L) != 1) { return false; }

    // Enable battery checks
    sendAT(GF("+CBATCHK=1"));
    if (waitResponse() != 1) { return false; }

    SimStatus ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the sim
    if (ret != SIM_READY && pin != NULL && strlen(pin) > 0) {
      simUnlock(pin);
      return (getSimStatus() == SIM_READY);
    } else {
      // if the sim is ready, or it's locked but no pin has been provided,
      // return true
      return (ret == SIM_READY || ret == SIM_LOCKED);
    }
  }

  /*
   * Power functions
   */
 protected:
  // Follows the SIM70xx template

  /*
   * Generic network functions
   */
 protected:
<<<<<<< HEAD
  bool isNetworkConnectedImpl() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

 public:
  String getNetworkModes() {
    sendAT(GF("+CNMP=?"));
    if (waitResponse(GF(GSM_NL "+CNMP:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
  }

  String currentNetworkMode() {
    sendAT("+CNMP?");
    if (waitResponse(GF(GSM_NL "+CNMP:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
  }

  String setNetworkMode(uint8_t mode) {
    sendAT(GF("+CNMP="), mode);
    if (waitResponse(GF(GSM_NL "+CNMP:")) != 1) { return "OK"; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
  }

  String getPreferredModes() {
    sendAT(GF("+CMNB=?"));
    if (waitResponse(GF(GSM_NL "+CMNB:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
  }

  String currentPreferredMode() {
    sendAT("+CMNB?");
    if (waitResponse(GF(GSM_NL "+CMNB:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
  }

  String setPreferredMode(uint8_t mode) {
    sendAT(GF("+CMNB="), mode);
    if (waitResponse(GF(GSM_NL "+CMNB:")) != 1) { return "OK"; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
  }

=======
>>>>>>> 6e10a3a009d121b0b5ad2f04b9ecbea379d10170
  String getLocalIPImpl() {
    sendAT(GF("+CNACT?"));
    if (waitResponse(GF(GSM_NL "+CNACT:")) != 1) { return ""; }
    streamSkipUntil('\"');
    String res = stream.readStringUntil('\"');
    waitResponse();
    return res;
  }

  /*
   * GPRS functions
   */
 protected:
  bool gprsConnectImpl(const char* apn, const char* user = NULL,
                       const char* pwd = NULL) {
    gprsDisconnect();

<<<<<<< HEAD
    // Open data connection
    sendAT(GF("+CNACT=1,\""), apn, GF("\""));
    if (waitResponse(60000L) != 1) { return false; }

    // Set the Bearer for the IP
    sendAT(GF(
        "+SAPBR=3,1,\"Contype\",\"GPRS\""));  // Set the connection type to GPRS
=======
    // Bearer settings for applications based on IP
    // Set the connection type to GPRS
    sendAT(GF("+SAPBR=3,1,\"Contype\",\"GPRS\""));
>>>>>>> 6e10a3a009d121b0b5ad2f04b9ecbea379d10170
    waitResponse();

    // Set the APN
    sendAT(GF("+SAPBR=3,1,\"APN\",\""), apn, '"');
    waitResponse();

    // Set the user name
    if (user && strlen(user) > 0) {
      sendAT(GF("+SAPBR=3,1,\"USER\",\""), user, '"');
      waitResponse();
    }
    // Set the password
    if (pwd && strlen(pwd) > 0) {
      sendAT(GF("+SAPBR=3,1,\"PWD\",\""), pwd, '"');
      waitResponse();
    }

    // Define the PDP context
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    // Attach to GPRS
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    // Activate the PDP context
    sendAT(GF("+CGACT=1,1"));
    waitResponse(60000L);

    // Open the definied GPRS bearer context
    sendAT(GF("+SAPBR=1,1"));
    waitResponse(85000L);
    // Query the GPRS bearer context status
    sendAT(GF("+SAPBR=2,1"));
    if (waitResponse(30000L) != 1) { return false; }

<<<<<<< HEAD
    // Attach to GPRS
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    // TODO(?): wait AT+CGATT?

    // Check data connection

    sendAT(GF("+CNACT?"));
    if (waitResponse(GF(GSM_NL "+CNACT:")) != 1) { return false; }
    int res = streamGetIntBefore(',');
    waitResponse();
=======
    // Set the TCP application toolkit to multi-IP
    sendAT(GF("+CIPMUX=1"));
    if (waitResponse() != 1) { return false; }

    // Put the TCP application toolkit in "quick send" mode
    // (thus no extra "Send OK")
    sendAT(GF("+CIPQSEND=1"));
    if (waitResponse() != 1) { return false; }

    // Set the TCP application toolkit to get data manually
    sendAT(GF("+CIPRXGET=1"));
    if (waitResponse() != 1) { return false; }

    // Start the TCP application toolkit task and set APN, USER NAME, PASSWORD
    sendAT(GF("+CSTT=\""), apn, GF("\",\""), user, GF("\",\""), pwd, GF("\""));
    if (waitResponse(60000L) != 1) { return false; }

    // Bring up the TCP application toolkit wireless connection with GPRS or CSD
    sendAT(GF("+CIICR"));
    if (waitResponse(60000L) != 1) { return false; }

    // Get local IP address for the TCP application toolkit
    // only assigned after connection
    sendAT(GF("+CIFSR;E0"));
    if (waitResponse(10000L) != 1) { return false; }
>>>>>>> 6e10a3a009d121b0b5ad2f04b9ecbea379d10170

    return res == 1;
  }

  bool gprsDisconnectImpl() {
<<<<<<< HEAD
    // Shut the TCP/IP connection
    // CNACT will close *all* open connections
    sendAT(GF("+CNACT=0"));
=======
    // Shut the TCP application toolkit connection
    // CIPSHUT will close *all* open TCP application toolkit connections
    sendAT(GF("+CIPSHUT"));
>>>>>>> 6e10a3a009d121b0b5ad2f04b9ecbea379d10170
    if (waitResponse(60000L) != 1) { return false; }

    sendAT(GF("+CGATT=0"));  // Deactivate the bearer context
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  /*
   * SIM card functions
   */
 protected:
  // Follows the SIM70xx template

  /*
   * Messaging functions
   */
 protected:
  // Follows all messaging functions per template

  /*
   * GPS/GNSS/GLONASS location functions
   */
 protected:
  // Follows the SIM70xx template

  /*
   * Time functions
   */
  // Can follow CCLK as per template

  /*
   * NTP server functions
   */
  // Can sync with server using CNTP as per template

  /*
   * Battery functions
   */
 protected:
  // Follows all battery functions per template

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75) {
<<<<<<< HEAD
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;

    sendAT(GF("+CACID="), mux);
    if (waitResponse(timeout_ms) != 1) return false;

    if (ssl) {
      sendAT(GF("+CSSLCFG=\"sslversion\",0,3"));  // TLS 1.2
      waitResponse();

      sendAT(GF("+CSSLCFG=\"ctxindex\",0"));
      waitResponse();
    }

    sendAT(GF("+CASSLCFG="), mux, ',', GF("ssl,"), ssl);
    waitResponse();

    sendAT(GF("+CASSLCFG="), mux, ',', GF("protocol,0"));
    waitResponse();

    sendAT(GF("+CAOPEN="), mux, ',', GF("\""), host, GF("\","), port);

    if (waitResponse(timeout_ms, GF(GSM_NL "+CAOPEN:")) != 1) { return 0; }
    streamSkipUntil(',');  // Skip mux

    int8_t res = streamGetIntBefore('\n');

    return 0 == res;
=======
    if (ssl) { DBG("SSL only supported using application on SIM7000!"); }
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;

    // when not using SSL, the TCP application toolkit is more stable
    sendAT(GF("+CIPSTART="), mux, ',', GF("\"TCP"), GF("\",\""), host,
           GF("\","), port);
    return (1 ==
            waitResponse(timeout_ms, GF("CONNECT OK" GSM_NL),
                         GF("CONNECT FAIL" GSM_NL),
                         GF("ALREADY CONNECT" GSM_NL), GF("ERROR" GSM_NL),
                         GF("CLOSE OK" GSM_NL)));
>>>>>>> 6e10a3a009d121b0b5ad2f04b9ecbea379d10170
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CASEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }

    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();

<<<<<<< HEAD
    if (waitResponse(GF(GSM_NL "+CASEND:")) != 1) { return 0; }
    streamSkipUntil(',');                            // Skip mux
    if (streamGetIntBefore(',') != 0) { return 0; }  // If result != success
=======
    if (waitResponse(GF(GSM_NL "DATA ACCEPT:")) != 1) { return 0; }
    streamSkipUntil(',');  // Skip mux
>>>>>>> 6e10a3a009d121b0b5ad2f04b9ecbea379d10170
    return streamGetIntBefore('\n');
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!sockets[mux]) return 0;

<<<<<<< HEAD
    sendAT(GF("+CARECV="), mux, ',', (uint16_t)size);

    if (waitResponse(GF("+CARECV:")) != 1) {
      sockets[mux]->sock_available = 0;
      return 0;
    }

    stream.read();
    if (stream.peek() == '0') {
      waitResponse();
      sockets[mux]->sock_available = 0;
      return 0;
    }

    int16_t len_confirmed = streamGetIntBefore(',');

    for (int i = 0; i < size; i++) {
=======
#ifdef TINY_GSM_USE_HEX
    sendAT(GF("+CIPRXGET=3,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#else
    sendAT(GF("+CIPRXGET=2,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#endif
    streamSkipUntil(',');  // Skip Rx mode 2/normal or 3/HEX
    streamSkipUntil(',');  // Skip mux
    int16_t len_requested = streamGetIntBefore(',');
    //  ^^ Requested number of data bytes (1-1460 bytes)to be read
    int16_t len_confirmed = streamGetIntBefore('\n');
    // ^^ Confirmed number of data bytes to be read, which may be less than
    // requested. 0 indicates that no data can be read.
    // SRGD NOTE:  Contrary to above (which is copied from AT command manual)
    // this is actually be the number of bytes that will be remaining in the
    // buffer after the read.
    for (int i = 0; i < len_requested; i++) {
>>>>>>> 6e10a3a009d121b0b5ad2f04b9ecbea379d10170
      uint32_t startMillis = millis();
      while (!stream.available() &&
             (millis() - startMillis < sockets[mux]->_timeout)) {
        TINY_GSM_YIELD();
      }
      char c = stream.read();
      sockets[mux]->rx.put(c);
    }
    // DBG("### READ:", len_requested, "from", mux);
    // sockets[mux]->sock_available = modemGetAvailable(mux);
    sockets[mux]->sock_available = len_confirmed;
    return len_confirmed;
  }

  size_t modemGetAvailable(uint8_t mux) {
    if (!sockets[mux]) return 0;

<<<<<<< HEAD
    if (!sockets[mux]->sock_connected) {
      sockets[mux]->sock_connected = modemGetConnected(mux);
    }
    if (!sockets[mux]->sock_connected) return 0;

    sendAT(GF("+CARECV?"));

    int8_t readMux = -1;
    while (readMux != mux) {
      if (waitResponse(GF("+CARECV:")) != 1) {
        sockets[mux]->sock_connected = modemGetConnected(mux);
        return 0;
      };
      readMux = streamGetIntBefore(',');
=======
    sendAT(GF("+CIPRXGET=4,"), mux);
    size_t result = 0;
    if (waitResponse(GF("+CIPRXGET:")) == 1) {
      streamSkipUntil(',');  // Skip mode 4
      streamSkipUntil(',');  // Skip mux
      result = streamGetIntBefore('\n');
      waitResponse();
>>>>>>> 6e10a3a009d121b0b5ad2f04b9ecbea379d10170
    }

    size_t result = streamGetIntBefore('\n');
    waitResponse();

    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    sendAT(GF("+CASTATE?"));
    int8_t readMux = -1;
    while (readMux != mux) {
      if (waitResponse(GF("+CASTATE:")) != 1) return 0;
      readMux = streamGetIntBefore(',');
    }
    int8_t res = streamGetIntBefore('\n');
    waitResponse();
    return 1 == res;
  }

 public:
  bool modemGetConnected(const char* host, uint16_t port, uint8_t mux) {
    sendAT(GF("+CAOPEN?"));
    int8_t readMux = -1;
    while (readMux != mux) {
      if (waitResponse(GF("+CAOPEN:")) != 1) return 0;
      readMux = streamGetIntBefore(',');
    }
    streamSkipUntil('\"');

    size_t hostLen = strlen(host);

    char buffer[hostLen];
    stream.readBytesUntil('\"', buffer, hostLen);
    streamSkipUntil(',');
    uint16_t connectedPort = streamGetIntBefore('\n');
    waitResponse();
    bool samePort                = connectedPort == port;
    bool sameHost                = memcmp(buffer, host, hostLen) == 0;
    sockets[mux]->sock_connected = sameHost && samePort;

    return sockets[mux]->sock_connected;
  }

  /*
   * Utilities
   */
 public:
  // TODO(vshymanskyy): Optimize this!
  int8_t waitResponse(uint32_t timeout_ms, String& data,
                      GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL) {
    /*String r1s(r1); r1s.trim();
    String r2s(r2); r2s.trim();
    String r3s(r3); r3s.trim();
    String r4s(r4); r4s.trim();
    String r5s(r5); r5s.trim();
    DBG("### ..:", r1s, ",", r2s, ",", r3s, ",", r4s, ",", r5s);*/
    data.reserve(64);
    uint8_t  index       = 0;
    uint32_t startMillis = millis();
    do {
      TINY_GSM_YIELD();
      while (stream.available() > 0) {
        TINY_GSM_YIELD();
        int8_t a = stream.read();
        if (a <= 0) continue;  // Skip 0x00 bytes, just in case
        data += static_cast<char>(a);
        if (r1 && data.endsWith(r1)) {
          index = 1;
          goto finish;
        } else if (r2 && data.endsWith(r2)) {
          index = 2;
          goto finish;
        } else if (r3 && data.endsWith(r3)) {
#if defined TINY_GSM_DEBUG
          if (r3 == GFP(GSM_CME_ERROR)) {
            streamSkipUntil('\n');  // Read out the error
          }
#endif
          index = 3;
          goto finish;
        } else if (r4 && data.endsWith(r4)) {
          index = 4;
          goto finish;
        } else if (r5 && data.endsWith(r5)) {
          index = 5;
          goto finish;
        } else if (data.endsWith(GF(GSM_NL "+CIPRXGET:"))) {
          int8_t mode = streamGetIntBefore(',');
          if (mode == 1) {
            int8_t mux = streamGetIntBefore('\n');
            if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
              sockets[mux]->got_data = true;
            }
            data = "";
            // DBG("### Got Data:", mux);
          } else {
            data += mode;
          }
        } else if (data.endsWith(GF(GSM_NL "+CARECV:"))) {
          int8_t mux = streamGetIntBefore(',');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->got_data = true;
          }
          data = "";
          // DBG("### Got Data:", mux);
        } else if (data.endsWith(GF(GSM_NL "+RECEIVE:"))) {
          int8_t  mux = streamGetIntBefore(',');
          int16_t len = streamGetIntBefore('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->got_data = true;
            if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
          }
          data = "";
          // DBG("### Got Data:", len, "on", mux);
        } else if (data.endsWith(GF("CLOSED" GSM_NL))) {
          int8_t nl   = data.lastIndexOf(GSM_NL, data.length() - 8);
          int8_t coma = data.indexOf(',', nl + 2);
          int8_t mux  = data.substring(nl + 2, coma).toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->sock_connected = false;
          }
          data = "";
          DBG("### Closed: ", mux);
        } else if (data.endsWith(GF("*PSNWID:"))) {
          streamSkipUntil('\n');  // Refresh network name by network
          data = "";
          DBG("### Network name updated.");
        } else if (data.endsWith(GF("*PSUTTZ:"))) {
          streamSkipUntil('\n');  // Refresh time and time zone by network
          data = "";
          DBG("### Network time and time zone updated.");
        } else if (data.endsWith(GF("+CTZV:"))) {
          streamSkipUntil('\n');  // Refresh network time zone by network
          data = "";
          DBG("### Network time zone updated.");
        } else if (data.endsWith(GF("DST: "))) {
          streamSkipUntil(
              '\n');  // Refresh Network Daylight Saving Time by network
          data = "";
          DBG("### Daylight savings time state updated.");
        } else if (data.endsWith(GF(GSM_NL "SMS Ready" GSM_NL))) {
          data = "";
          DBG("### Unexpected module reset!");
          init();
        }
      }
    } while (millis() - startMillis < timeout_ms);
  finish:
    if (!index) {
      data.trim();
      if (data.length()) { DBG("### Unhandled:", data); }
      data = "";
    }
    // data.replace(GSM_NL, "/");
    // DBG('<', index, '>', data);
    return index;
  }

  int8_t waitResponse(uint32_t timeout_ms, GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL) {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
  }

  int8_t waitResponse(GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL) {
    return waitResponse(1000, r1, r2, r3, r4, r5);
  }

 protected:
  GsmClientSim7000* sockets[TINY_GSM_MUX_COUNT];
};

#endif  // SRC_TINYGSMCLIENTSIM7000_H_
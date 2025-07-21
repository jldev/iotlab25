#include "WiFi.h"
#include <esp_wifi.h>

void send(const uint8_t *buf, uint32_t len) {
    for (int i=0; i<len; i++) {
        printf("%02x", buf[i]);
    }
}

void send_int(uint32_t i) {
    send((uint8_t*)&i, 4);
}

void send_packet(uint32_t ts_sec, uint32_t ts_usec, uint32_t len, const uint8_t* buf) {
    uint32_t incl_len = len > 2000 ? 2000 : len;

    printf("Packet % 3d.%06d %d %d\n", ts_sec, ts_usec, incl_len, len);

    printf("DATA:");
    send_int(ts_sec);
    send_int(ts_usec);
    send_int(incl_len);
    send_int(len);
    send(buf, incl_len);
    printf("\n");
}

void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
    const wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    const wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)pkt->rx_ctrl;

    int64_t t = esp_timer_get_time();
    send_packet(t/1000000, t%1000000, ctrl.sig_len-4, pkt->payload);
}

wifi_promiscuous_filter_t filter = {
    .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL,
};

void setup() {
  Serial.begin(921600);
  WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_channel(7, WIFI_SECOND_CHAN_BELOW);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&sniffer_callback);
  esp_wifi_start();
}

void loop() {
  // Nothing to do here - everything happens in the callback
}


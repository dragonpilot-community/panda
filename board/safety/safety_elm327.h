const int GM_CAMERA_DIAG_ADDR = 0x24B;

static bool elm327_tx_hook(const CANPacket_t *to_send) {
  bool tx = true;
  int addr = GET_ADDR(to_send);
  int len = GET_LEN(to_send);

  // All ISO 15765-4 messages must be 8 bytes long
  if (len != 8) {
    tx = false;
  }

  // Check valid 29 bit send addresses for ISO 15765-4
  // Check valid 11 bit send addresses for ISO 15765-4
  if ((addr != 0x18DB33F1) && ((addr & 0x1FFF00FF) != 0x18DA00F1) &&
      ((addr & 0x1FFFFF00) != 0x600) && ((addr & 0x1FFFFF00) != 0x700) &&
      (addr != GM_CAMERA_DIAG_ADDR)) {
    tx = false;
  }

  // GM camera uses non-standard diagnostic address, this has no control message address collisions
  if ((addr == GM_CAMERA_DIAG_ADDR) && (len == 8)) {
    // Only allow known frame types for ISO 15765-2
    if ((GET_BYTE(to_send, 0) & 0xF0U) > 0x30U) {
      tx = false;
    }
  }
  return tx;
}

static bool elm327_tx_lin_hook(int lin_num, uint8_t *data, int len) {
  int tx = 1;
  if (lin_num != 0) {
    tx = 0;  //Only operate on LIN 0, aka serial 2
  }
  if ((len < 5) || (len > 11)) {
    tx = 0;  //Valid KWP size
  }
  if (!(((data[0] & 0xF8U) == 0xC0U) && ((data[0] & 0x07U) != 0U) &&
        (data[1] == 0x33U) && (data[2] == 0xF1U))) {
    tx = 0;  //Bad msg
  }
  return tx;
}

// If current_board->has_obd and safety_param == 0, bus 1 is multiplexed to the OBD-II port
const safety_hooks elm327_hooks = {
  .init = nooutput_init,
  .rx = default_rx_hook,
  .tx = elm327_tx_hook,
  .tx_lin = elm327_tx_lin_hook,
  .fwd = default_fwd_hook,
};

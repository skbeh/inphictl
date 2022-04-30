#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "libusb.h"

#define TIMEOUT 200
#define SETUP_LENGTH 8
#define COMMAND_LENGTH 8

#define LOCK_FILE_PATH "/run/inphictl.lock"

#define MOUSE_VID 0x18f8
#define MOUSE_PID 0x1286

#define REQUEST_TYPE                                                           \
  (LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT)

#define W_VALUE 0x307
#define W_INDEX 0x1

enum LIGHT_CONFIG {
  LIGHT_CONTROL_POSITION = 3,
  LIGHT_ON = 0x10,
  LIGHT_OFF = 0x17
};

enum DPI_CONFIG {
  DPI_CONTROL_POSITION = 2,
  DPI_1200 = 0x3f,
  DPI_1600,
  DPI_2400,
  DPI_3200,
  DPI_4800
};

int main(int argc, char *argv[]) {
  int r;
  FILE *fp;
  libusb_device_handle *devh;
  uint8_t light_command[COMMAND_LENGTH] = {0x07, 0x13, 0x00, 0x00, 0x0e};
  uint8_t dpi_command[COMMAND_LENGTH] = {0x07, 0x09, 0x00, 0x0e};

  light_command[LIGHT_CONTROL_POSITION] = LIGHT_OFF;
  dpi_command[DPI_CONTROL_POSITION] = DPI_4800;

  fp = fopen(LOCK_FILE_PATH, "r");
  if (fp) {
    printf("locked\n");
    fclose(fp);
    return 1;
  } else {
    fp = fopen(LOCK_FILE_PATH, "w");
    if (fp) {
      fclose(fp);
    } else {
      fprintf(stderr, "failed to create lock file\n");
    }
  }

  r = libusb_init(NULL);
  if (r < 0) {
    fprintf(stderr, "failed to initialise libusb %d - %s\n", r,
            libusb_strerror(r));
    return 1;
  }

  devh = libusb_open_device_with_vid_pid(NULL, MOUSE_VID, MOUSE_PID);
  if (!devh) {
    fprintf(stderr, "could not find/open device\n");
    return 1;
  }

  libusb_set_auto_detach_kernel_driver(devh, 1);

  r = libusb_claim_interface(devh, 0);
  if (r < 0) {
    fprintf(stderr, "claim interface 0 error %d - %s\n", r, libusb_strerror(r));
    goto out;
  }
  r = libusb_claim_interface(devh, 1);
  if (r < 0) {
    fprintf(stderr, "claim interface 1 error %d - %s\n", r, libusb_strerror(r));
    goto out_release;
  }

  r = libusb_control_transfer(
      devh, REQUEST_TYPE, LIBUSB_REQUEST_SET_CONFIGURATION, W_VALUE, W_INDEX,
      light_command, sizeof(light_command), TIMEOUT);
  if (r < 0) {
    fprintf(stderr, "light transfer error %d - %s\n", r, libusb_strerror(r));
    goto out_release;
  }

  r = libusb_control_transfer(
      devh, REQUEST_TYPE, LIBUSB_REQUEST_SET_CONFIGURATION, W_VALUE, W_INDEX,
      dpi_command, sizeof(dpi_command), TIMEOUT);
  if (r < 0) {
    fprintf(stderr, "dpi transfer error %d - %s\n", r, libusb_strerror(r));
    goto out_release;
  }

out_release:
  libusb_release_interface(devh, 0);
  libusb_release_interface(devh, 1);
out:
  libusb_close(devh);
  libusb_exit(NULL);
  return abs(r);
}

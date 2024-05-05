/****************************************************************************
 * apps/testing/drivertest/drivertest_rtc.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/ioctl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>

#include <nuttx/timers/rtc.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define RTC_DEFAULT_DEVPATH "/dev/rtc0"

/****************************************************************************
 * Private Type
 ****************************************************************************/
char tz_info[60] = "UTC+8";

struct rtc_state_s
{
  char devpath[PATH_MAX];
  bool read_time;
  bool have_set_time;
  bool set_en;
  struct rtc_time set_time;
};
/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: show_usage
 ****************************************************************************/

static void show_usage(FAR const char *progname,
                       FAR struct rtc_state_s *rtc_state, int exitcode)
{
  printf("Usage: %s -d <devpath>\n", progname);
  printf("  [-d devpath] selects the RTC device.\n"
         "  Default: %s Current: %s\n", RTC_DEFAULT_DEVPATH,
         rtc_state->devpath);
  exit(exitcode);
}

/****************************************************************************
 * Name: parse_commandline
 ****************************************************************************/

static void parse_commandline(FAR struct rtc_state_s *rtc_state, int argc,
                              FAR char **argv)
{
  int ch;
  while ((ch = getopt(argc, argv, "p:rhsY:M:D:H:m:S:Z")) != ERROR)
    {
      switch (ch)
        {
          case 'p':
            // printf("%s %d\n",__func__,__LINE__);
            strlcpy(rtc_state->devpath, optarg, sizeof(rtc_state->devpath));
            printf("rtc_state->devpath = %s\n",rtc_state->devpath);
            break;
          case 'r':
            printf("Read rtc time!\n");
            rtc_state->read_time = 1;
            break;
          case 'h':
            printf("Get have set time status!\n");
            rtc_state->have_set_time = 1;
            break;
          case 's':
            printf("Now set time!\n");
            rtc_state->set_en = 1;
            break;
          case 'Y':
            // printf("Please input year in 1970~2038\n");
            rtc_state->set_time.tm_year = atoi(optarg)-1900;
            // printf("Now year be set to %d!\n",rtc_state->set_time.tm_year);
            break;
          case 'M':
            // printf("Please input month in 1~12\n");
            rtc_state->set_time.tm_mon = atoi(optarg);
            // printf("Now month be set to %d!\n",rtc_state->set_time.tm_mon);
            break;
          case 'D':
            // printf("Please input day in 1~31\n");
            rtc_state->set_time.tm_mday = atoi(optarg);
            // printf("Now day be set to %d!\n",rtc_state->set_time.tm_mday);
            break;
          case 'H':
            // printf("Please input hour in 0~23\n");
            rtc_state->set_time.tm_hour = atoi(optarg);
            // printf("Now hour be set to %d!\n",rtc_state->set_time.tm_hour);
            break;
          case 'm':
            // printf("Please input minute in 0~60\n");
            rtc_state->set_time.tm_min = atoi(optarg);
            // printf("Now minute be set to %d!\n",rtc_state->set_time.tm_min);
            break;
          case 'S':
            // printf("Please input second in 0~60\n");
            rtc_state->set_time.tm_sec = atoi(optarg);
            // printf("Now second be set to %d!\n",rtc_state->set_time.tm_sec);
            break;
           case 'Z':
            rtc_state->set_time.tm_zone = tz_info;
            rtc_state->set_time.tm_gmtoff = 8;
            break;
          case '?':
            printf("Unsupported option: %s\n", optarg);
            show_usage(argv[0], rtc_state, EXIT_FAILURE);
        }
    }
}

/****************************************************************************
 * Name: test_case_rtc
 ****************************************************************************/
static int test_case_rtc (FAR void *state)
{
  int fd;
  int ret = 0;
  bool have_set_time;
  char timbuf[64];
  // struct rtc_time set_time;
  struct rtc_time rd_time;
  FAR struct rtc_state_s *rtc_state;

  rtc_state = (FAR struct rtc_state_s *)state;
  // printf("rtc_state->devpath = %s rtc_state->read_time = %d\n",rtc_state->devpath,rtc_state->read_time);
  fd = open(rtc_state->devpath, O_WRONLY);
  if (fd < 0) {
    printf("open /dev/rtc0 failed!\n");
    return -1;
  }
  if (rtc_state->have_set_time == true) {
    ret = ioctl(fd, RTC_HAVE_SET_TIME,
                (unsigned long)((uintptr_t)&have_set_time));
    if (ret < 0){
      printf("RTC_HAVE_SET_TIME cmd failed!\n");
      return -1;
    }
    printf("have_set_time:%d\n",have_set_time);
  }

  if (rtc_state->set_en == true) {
    printf("Setting time!\n");
    ret = strftime(timbuf, sizeof(timbuf), "%a, %b %d %H:%M:%S %Y",
                  (FAR struct tm *)&(rtc_state->set_time));
    printf("%s\n",timbuf);
    ret = ioctl(fd, RTC_SET_TIME, (unsigned long)((uintptr_t)&(rtc_state->set_time)));
    if (ret < 0){
      printf("RTC_SET_TIME cmd failed!\n");
      return -1;
    }
  }
  // printf("%s %d\n",__func__,__LINE__);
  if (rtc_state->read_time == true) {
    ret = ioctl(fd, RTC_RD_TIME, (unsigned long)((uintptr_t)&rd_time));
    if (ret < 0){
      close(fd);
      printf("RTC_RD_TIME cmd failed!\n");
      return -1;
    }
    // printf("%d-%d-%d\n",rd_time.tm_year,rd_time.tm_mon,rd_time.tm_mday);
    ret = strftime(timbuf, sizeof(timbuf), "%a, %b %d %H:%M:%S %Y",
                 (FAR struct tm *)&rd_time);
    if (ret < 0){
      printf("strftime failed!\n");
      close(fd);
      return -1;
    }
    printf("tm_zone:%s\n",rd_time.tm_zone);
    printf("%s\n",timbuf);
  }
  
  // printf("%s %d\n",__func__,__LINE__);

  
  // printf("%s %d\n",__func__,__LINE__);
// close_fd:
  close(fd);
  return ret;
}
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main
 ****************************************************************************/
int main(int argc, FAR char *argv[])
{
  struct rtc_state_s rtc_state =
  {
    .devpath = RTC_DEFAULT_DEVPATH,
    .read_time = 0,
  };

  parse_commandline(&rtc_state, argc, argv);

  test_case_rtc(&rtc_state);

  return 0;
}

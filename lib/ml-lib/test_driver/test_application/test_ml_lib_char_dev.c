// SPDX-License-Identifier: GPL-2.0
/*
 * Machine Learning (ML) library
 *
 * Test program for ml_lib_dev testing driver
 *
 * Copyright (C) 2025-2026 Viacheslav Dubeyko <slava@dubeyko.com>
 *
 * Compile with: gcc -o test_ml_lib_char_dev test_ml_lib_char_dev.c
 * Run with:     sudo ./test_ml_lib_char_dev
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "ml_lib_char_dev_ioctl.h"

#define DEVICE_PATH "/dev/mllibdev"
#define SYSFS_BASE "/sys/class/ml_lib_test/mllibdev"
#define PROC_PATH "/proc/mllibdev"

static void print_separator(const char *title)
{
	printf("\n========== %s ==========\n", title);
}

static void read_sysfs_attr(const char *attr_name)
{
	char path[256];
	char buffer[256];
	FILE *fp;

	snprintf(path, sizeof(path), "%s/%s", SYSFS_BASE, attr_name);
	fp = fopen(path, "r");
	if (!fp) {
		perror("Failed to open sysfs attribute");
		return;
	}

	if (fgets(buffer, sizeof(buffer), fp)) {
		printf("  %s: %s", attr_name, buffer);
	}

	fclose(fp);
}

static void show_sysfs_info(void)
{
	print_separator("Sysfs Attributes");
	read_sysfs_attr("buffer_size");
	read_sysfs_attr("data_size");
	read_sysfs_attr("access_count");
	printf("\n");
	read_sysfs_attr("stats");
}

static void show_proc_info(void)
{
	char buffer[1024];
	FILE *fp;

	print_separator("Procfs Information");

	fp = fopen(PROC_PATH, "r");
	if (!fp) {
		perror("Failed to open procfs entry");
		return;
	}

	while (fgets(buffer, sizeof(buffer), fp)) {
		printf("%s", buffer);
	}

	fclose(fp);
}

static void test_write(int fd)
{
	const char *test_data = "Hello from userspace! This is a test of the mllibdev driver.";
	ssize_t ret;

	print_separator("Write Test");

	ret = write(fd, test_data, strlen(test_data));
	if (ret < 0) {
		perror("Write failed");
		return;
	}

	printf("Successfully wrote %zd bytes\n", ret);
	printf("Data: \"%s\"\n", test_data);
}

static void test_read(int fd)
{
	char buffer[256];
	ssize_t ret;

	print_separator("Read Test");

	/* Seek to beginning */
	lseek(fd, 0, SEEK_SET);

	ret = read(fd, buffer, sizeof(buffer) - 1);
	if (ret < 0) {
		perror("Read failed");
		return;
	}

	buffer[ret] = '\0';
	printf("Successfully read %zd bytes\n", ret);
	printf("Data: \"%s\"\n", buffer);
}

static void test_ioctl(int fd)
{
	int size;
	int ret;

	print_separator("IOCTL Tests");

	/* Get current size */
	ret = ioctl(fd, ML_LIB_TEST_DEV_IOCGETSIZE, &size);
	if (ret < 0) {
		perror("IOCTL GETSIZE failed");
		return;
	}
	printf("Current data size: %d bytes\n", size);

	/* Set new size */
	size = 50;
	ret = ioctl(fd, ML_LIB_TEST_DEV_IOCSETSIZE, &size);
	if (ret < 0) {
		perror("IOCTL SETSIZE failed");
		return;
	}
	printf("Set data size to: %d bytes\n", size);

	/* Verify new size */
	ret = ioctl(fd, ML_LIB_TEST_DEV_IOCGETSIZE, &size);
	if (ret < 0) {
		perror("IOCTL GETSIZE failed");
		return;
	}
	printf("Verified new size: %d bytes\n", size);

	/* Reset buffer */
	ret = ioctl(fd, ML_LIB_TEST_DEV_IOCRESET);
	if (ret < 0) {
		perror("IOCTL RESET failed");
		return;
	}
	printf("Buffer reset successfully\n");

	/* Verify size after reset */
	ret = ioctl(fd, ML_LIB_TEST_DEV_IOCGETSIZE, &size);
	if (ret < 0) {
		perror("IOCTL GETSIZE failed");
		return;
	}
	printf("Size after reset: %d bytes\n", size);
}

int main(void)
{
	int fd;

	printf("ML Library Testing Device Driver Test Program\n");
	printf("==================================\n");

	/* Open the device */
	fd = open(DEVICE_PATH, O_RDWR);
	if (fd < 0) {
		perror("Failed to open device");
		printf("\nMake sure:\n");
		printf("1. The driver module is loaded (lsmod | grep mllibdev)\n");
		printf("2. You have proper permissions (run with sudo)\n");
		printf("3. The device node exists (ls -l /dev/mllibdev)\n");
		return 1;
	}

	printf("Device opened successfully: %s\n", DEVICE_PATH);

	/* Run tests */
	test_write(fd);
	test_read(fd);
	test_ioctl(fd);

	/* Show sysfs and proc information */
	show_sysfs_info();
	show_proc_info();

	/* Final stats */
	print_separator("Final Test");
	printf("All tests completed successfully!\n\n");

	/* Close the device */
	close(fd);

	return 0;
}

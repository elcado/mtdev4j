/*
 * mtdev4j.c
 *
 *  Created on: 24 oct. 2012
 *      Author: Frédéric Cadier
 */

#include <stdio.h>
#include <stdarg.h>
#include <jni.h>
#include <mtdev.h>
#include <unistd.h>
#include <fcntl.h>

#include "../MTDevInputSource/bin/org_mt4j_input_inputSources_MTDevice.h"

void loadDeviceCaps(JNIEnv *env, jclass clazz, jobject instance);
void devClose();

int fd;
struct mtdev dev;

void info(char *fmt, ...) {
	va_list argp;

	fprintf(stdout, "libmtdev4j.so: ");
	va_start(argp, fmt);
	vfprintf(stdout, fmt, argp);
	va_end(argp);
	fprintf(stdout, "\n");
}

void warn(char *fmt, ...) {
	va_list argp;

	fprintf(stderr, "libmtdev4j.so: ");
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
	fprintf(stderr, "\n");
}

#define TEST_ENV_EXCEPTION(env)			\
		if ((*env)->ExceptionOccurred(env)) { \
			(*env)->ExceptionDescribe(env); \
		}

#define TEST_ENV_EXCEPTION_RET(env)			\
		if ((*env)->ExceptionOccurred(env)) { \
			(*env)->ExceptionDescribe(env); \
			devClose(); \
			return NULL ; \
		}

#define CHECK(dev, name)			\
	if (mtdev_has_mt_event(dev, name))	\
		info("   %s [min=%d; max=%d; fuzz: %d; res:%d]", #name, \
				mtdev_get_abs_minimum(dev, name), \
				mtdev_get_abs_maximum(dev, name), \
				mtdev_get_abs_fuzz(dev, name), \
				mtdev_get_abs_resolution(dev, name))

JNIEXPORT jobject JNICALL Java_org_mt4j_input_inputSources_MTDevice_openDevice(
		JNIEnv *env, jclass clazz, jstring filename) {
	// get native string from java
	const char *_filename = (*env)->GetStringUTFChars(env, filename, JNI_FALSE);

	// open device
	fd = open(_filename, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		warn("Could not open the device %s", _filename);
		return NULL;
	}

	// grab the device
	if (ioctl(fd, EVIOCGRAB, 1)) {
		warn("Could not grab the device %s", _filename);
		return NULL;
	}

	// open as an MT device
	// note: mtdev_open automatically calls mtdev_close on error
	int mtfd = mtdev_open(&dev, fd);
	if (mtfd) {
		warn("mtdev could not open device: %d", mtfd);
		return NULL;
	}

	info("Multi-touch device %s opened", _filename);

	// get MTDeviceCaps class default constructor
	jmethodID clazz_init = (*env)->GetMethodID(env, clazz, "<init>", "()V");
	TEST_ENV_EXCEPTION_RET(env);

	// build instance
	jobject instance = (*env)->NewObject(env, clazz, clazz_init);
	TEST_ENV_EXCEPTION_RET(env);

	// get device capacities
	loadDeviceCaps(env, clazz, instance);

	// release native string
	(*env)->ReleaseStringUTFChars(env, filename, _filename);

	return instance;
}

#define SET_ABS_FIELD_VALUE(dev, clazz, instance, name)			\
		if (mtdev_has_mt_event(&dev, name)) { \
			fieldId = (*env)->GetFieldID(env, clazz, "is_"#name, "Z"); \
			TEST_ENV_EXCEPTION(env); \
			(*env)->SetIntField(env, instance, fieldId, JNI_TRUE); \
			TEST_ENV_EXCEPTION(env); \
			fieldId = (*env)->GetFieldID(env, clazz, #name, "I"); \
			TEST_ENV_EXCEPTION(env); \
			(*env)->SetIntField(env, instance, fieldId, mtdev_get_abs_maximum(&dev, name)); \
			TEST_ENV_EXCEPTION(env); \
		}

void loadDeviceCaps(JNIEnv *env, jclass clazz, jobject instance) {
	jfieldID fieldId;
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_SLOT);
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_TOUCH_MAJOR);
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_TOUCH_MINOR);
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_WIDTH_MAJOR);
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_WIDTH_MINOR);
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_ORIENTATION);
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_POSITION_X);
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_POSITION_Y);
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_TOOL_TYPE);
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_BLOB_ID);
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_TRACKING_ID);
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_PRESSURE);
	SET_ABS_FIELD_VALUE(dev, clazz, instance, ABS_MT_DISTANCE);

	return;
}

JNIEXPORT void JNICALL Java_org_mt4j_input_inputSources_MTDevice_closeDevice(
		JNIEnv *env, jobject this) {

	devClose();

	return;
}

void devClose() {
	// only closing something open
	if (fd < 0)
		return;

	// close mtdev device
	mtdev_close(&dev);

	// ungrab the device
	ioctl(fd, EVIOCGRAB, 0);

	// close device
	close(fd);

	info("File description #%d closed", fd);
}

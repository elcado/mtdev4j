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

#include "../MTDevInputSource/bin/org_mt4j_input_inputSources_MTDevInputSource.h"

//void devClose();

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

JNIEXPORT jboolean JNICALL Java_org_mt4j_input_inputSources_MTDevInputSource_openDevice(
		JNIEnv *env, jobject this, jstring filename) {
	// get native string from java
	const char *_filename = (*env)->GetStringUTFChars(env, filename, JNI_FALSE);

	// open device file
	fd = open(_filename, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		warn("Could not open the device %s", _filename);
		return JNI_FALSE;
	}

	// grab the device file
	if (ioctl(fd, EVIOCGRAB, 1)) {
		warn("Could not grab the device %s", _filename);
		return JNI_FALSE;
	}

	// open device through mtdev
	// note: mtdev_open automatically calls mtdev_close on error
	int mtfd = mtdev_open(&dev, fd);
	if (mtfd) {
		warn("mtdev could not open device: %d", mtfd);
		return JNI_FALSE;
	}

	// release native string
	(*env)->ReleaseStringUTFChars(env, filename, _filename);

	return JNI_TRUE;
}

#define TEST_ENV_EXCEPTION(env)			\
	/* test JNI env for occured exception */ \
	if ((*env)->ExceptionOccurred(env)) { \
		(*env)->ExceptionDescribe(env); \
	}

#define SET_ABS_FIELD_VALUE(dev, clazz, instance, name)			\
		/* test that dev has #name capability */ \
		if (mtdev_has_mt_event(&dev, name)) { \
			/* get capability flag in java class */ \
			fieldId = (*env)->GetFieldID(env, clazz, "is_"#name, "Z"); \
			TEST_ENV_EXCEPTION(env); \
			/* set flag to true */ \
			(*env)->SetIntField(env, instance, fieldId, JNI_TRUE); \
			TEST_ENV_EXCEPTION(env); \
			/* get capability attribute in java class */ \
			fieldId = (*env)->GetFieldID(env, clazz, #name, "I"); \
			TEST_ENV_EXCEPTION(env); \
			/* set capability attribute value */ \
			(*env)->SetIntField(env, instance, fieldId, mtdev_get_abs_maximum(&dev, name)); \
			TEST_ENV_EXCEPTION(env); \
		}

JNIEXPORT void JNICALL Java_org_mt4j_input_inputSources_MTDevInputSource_loadDeviceCaps(
		JNIEnv *env, jobject instance) {
	jclass clazz = (*env)->GetObjectClass(env, instance);
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

JNIEXPORT void JNICALL Java_org_mt4j_input_inputSources_MTDevInputSource_closeDevice(
		JNIEnv *env, jobject this) {
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
	return;
}

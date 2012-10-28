/*
 * Copyright Frédéric Cadier (2012)
 *
 * This software is a computer program whose purpose is to allow component programming in pure java.
 * 
 * This software is governed by the CeCILL-C license under French law and abiding by the rules of distribution of free software. You can use, modify
 * and/ or redistribute the software under the terms of the CeCILL-C license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 * 
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the license, users are provided only with
 * a limited warranty and the software's author, the holder of the economic rights, and the successive licensors have only limited liability.
 * 
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or developing or reproducing the software
 * by the user in light of its specific status of free software, that may mean that it is complicated to manipulate, and that also therefore means
 * that it is reserved for developers and experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and
 * test the software's suitability as regards their requirements in conditions enabling the security of their systems and/or data to be ensured and,
 * more generally, to use and operate it in the same conditions as regards security.
 * 
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL-C license and that you accept its terms.
 */

#include <stdio.h>
#include <jni.h>
#include <mtdev.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "mtdev4j_jni.h"
#include "mtdev4j.h"

int fd;
struct mtdev dev;

jboolean JNICALL Java_org_mt4j_input_inputSources_MTDevInputSource_openDevice(
		JNIEnv *env, jobject this, jstring filename) {
	// get native string from java
	const char *_filename = (*env)->GetStringUTFChars(env, filename, JNI_FALSE);
	TEST_ENV_EXCEPTION(env);

	// open device file
	fd = open(_filename, O_RDONLY);// | O_NONBLOCK);
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

	// set name attribute value in java class
	if (!loadDeviceName(env, this)) {
		warn("Could not get device name for %s", _filename);
		return JNI_FALSE;
	}

	// release native string
	(*env)->ReleaseStringUTFChars(env, filename, _filename);
	TEST_ENV_EXCEPTION(env);

	return JNI_TRUE;
}

int loadDeviceName(JNIEnv* env, jobject this) {
	// get device name
	char name[DEV_NAME_LENGTH] = "Unknown";
	if(! ioctl(fd, EVIOCGNAME(sizeof(name)-1), name)) {
		return JNI_FALSE;
	}
    name[DEV_NAME_LENGTH-1]=0;  // make sure it is null-terminated

	jclass clazz = (*env)->GetObjectClass(env, this);

	// build String on name
	jstring jName = (*env)->NewStringUTF(env, name);
	TEST_ENV_EXCEPTION(env);

	// get name attribute setter from java class
	jmethodID methodId = (*env)->GetMethodID(env, clazz, "setDevName", "(Ljava/lang/String;)V");
	TEST_ENV_EXCEPTION(env);

	// set class attribute value
	(*env)->CallVoidMethod(env, this, methodId, jName);
	TEST_ENV_EXCEPTION(env); \

	return JNI_TRUE;
}

#define ADD_ABS_MT_CAP(name)			\
		/* test that dev has #name capability */ \
		if (mtdev_has_mt_event(&dev, name)) { \
			(*env)->CallVoidMethod(env, instance, methodId, name, mtdev_get_abs_minimum(&dev, name), mtdev_get_abs_maximum(&dev, name)); \
			TEST_ENV_EXCEPTION(env); \
		}

JNIEXPORT void JNICALL Java_org_mt4j_input_inputSources_MTDevInputSource_loadDeviceCaps(
		JNIEnv *env, jobject instance) {
	// get instance clazz
	jclass clazz = (*env)->GetObjectClass(env, instance);

	// get ABS_MT cap setter method in java class
	jmethodID methodId = (*env)->GetMethodID(env, clazz, "addCap", "(III)V");
	TEST_ENV_EXCEPTION(env);

	// set values
	ADD_ABS_MT_CAP(ABS_MT_SLOT);
	ADD_ABS_MT_CAP(ABS_MT_TOUCH_MAJOR);
	ADD_ABS_MT_CAP(ABS_MT_TOUCH_MINOR);
	ADD_ABS_MT_CAP(ABS_MT_WIDTH_MAJOR);
	ADD_ABS_MT_CAP(ABS_MT_WIDTH_MINOR);
	ADD_ABS_MT_CAP(ABS_MT_ORIENTATION);
	ADD_ABS_MT_CAP(ABS_MT_POSITION_X);
	ADD_ABS_MT_CAP(ABS_MT_POSITION_Y);
	ADD_ABS_MT_CAP(ABS_MT_TOOL_TYPE);
	ADD_ABS_MT_CAP(ABS_MT_BLOB_ID);
	ADD_ABS_MT_CAP(ABS_MT_TRACKING_ID);
	ADD_ABS_MT_CAP(ABS_MT_PRESSURE);
	ADD_ABS_MT_CAP(ABS_MT_DISTANCE);

	return;
}

JNIEXPORT void JNICALL Java_org_mt4j_input_inputSources_MTDevInputSource_startEventLoop
  (JNIEnv *env, jobject this){
	struct input_event ev;

	// while the device has not been inactive for five seconds
	while (mtdev_get(&dev, fd, &ev, 1) > 0) {
		propagate_event(env, this, &ev);
	}
}

void propagate_event(JNIEnv *env, jobject this, const struct input_event *ev)
{
	static int slot;

	// get slot id
	if (ev->type == EV_ABS && ev->code == ABS_MT_SLOT) {
		slot = ev->value;
		return;
	}

	jclass clazz = (*env)->GetObjectClass(env, this);

	// get callback method in java class
	jmethodID methodId = (*env)->GetMethodID(env, clazz, "onMTDevTouch", "(IIII)V");
	TEST_ENV_EXCEPTION(env);

	// call callback
	(*env)->CallVoidMethod(env, this, methodId, slot, ev->type, ev->code, ev->value);
	TEST_ENV_EXCEPTION(env);

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

	return;
}

void warn(char *fmt, ...) {
	va_list argp;

	fprintf(stderr, "libmtdev4j.so: ");
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
	fprintf(stderr, "\n");
}

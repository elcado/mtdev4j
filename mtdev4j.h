/*
 * mtdev4j.h
 *
 *  Created on: 26 oct. 2012
 *      Author: Frédéric Cadier
 */

#ifndef MTDEV4J_H_
#define MTDEV4J_H_

// max length of device name
#define DEV_NAME_LENGTH 80

/* year-proof millisecond event time */
typedef __u64 mstime_t;

void info(char *fmt, ...);
void warn(char *fmt, ...);
int loadDeviceName(JNIEnv* env, jobject this);
//void loadDeviceName(JNIEnv* env, jobject this, char name[DEV_NAME_LENGTH]);
void propagate_event(JNIEnv *env, jobject this, const struct input_event *ev);

/*
 * MACROS
 */

// test JNI env for exception and render it
#define TEST_ENV_EXCEPTION(env)			\
	/* test JNI env for occured exception */ \
	if ((*env)->ExceptionOccurred(env)) { \
	    (*env)->ExceptionDescribe(env); /* write exception data to the console */ \
	    (*env)->ExceptionClear(env);    /* clear the exception that was pending */ \
	}

// set field value in java instance
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


#endif /* MTDEV4J_H_ */

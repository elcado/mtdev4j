/*
 * Copyright 2012 Frédéric Cadier <f.cadier@free.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
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

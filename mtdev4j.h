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

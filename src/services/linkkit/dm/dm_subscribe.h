#ifndef _IOTX_DM_SUBSCRIBE_H_
#define _IOTX_DM_SUBSCRIBE_H_

int iotx_dsub_multi(_IN_ char **subscribe, _IN_ int count);

int iotx_dsub_multi_next(_IN_ int devid, _IN_ int index);

int iotx_dsub_shadow_create(int devid);

int iotx_dsub_shadow_destroy(int devid);

int iotx_dsub_shadow_next(int devid, int index);

int iotx_dsub_local_register(void);

#endif

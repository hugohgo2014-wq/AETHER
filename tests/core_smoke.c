#include "aether/aether_core.h"
#include "aether/aether_plugin.h"
#include <assert.h>
#include <stdio.h>
static int loaded=0,updated=0,unloaded=0;
static int on_load(const AetherHostApi *host){assert(host&&host->abi_version==AETHER_PLUGIN_ABI_VERSION);loaded++;return 1;}
static void on_update(const AetherHostApi *host,float dt){assert(host&&dt>=0.0f);updated++;}
static void on_unload(const AetherHostApi *host){assert(host);unloaded++;}
int main(void){
 aether_core_init();double before=aether_time_seconds();
 void *p=aether_alloc(128);assert(p);p=aether_realloc(p,256);assert(p);aether_free(p);
 static const AetherPluginDescriptor d={AETHER_PLUGIN_ABI_VERSION,"test.smoke","Core smoke test",1u,on_load,on_update,on_unload};
 AetherPluginRegistry *r=aether_plugin_registry_create();assert(r);
 assert(aether_plugin_registry_register(r,&d)==AETHER_OK);
 assert(aether_plugin_registry_register(r,&d)==AETHER_ERROR_DUPLICATE);
 assert(aether_plugin_registry_count(r)==1u);aether_plugin_registry_update(r,1.0f);
 assert(loaded==1&&updated==1);assert(aether_time_seconds()>=before);
 aether_plugin_registry_destroy(r);assert(unloaded==1);aether_core_shutdown();
 puts("AETHER core smoke test passed");return 0;
}

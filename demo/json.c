/*added in 2022/0317*/


#include <stdlib.h>
#include <stdio.h>
#include "cjson.h"

//因为使用了palloc 所以不需要进行free，最后统一释放
const char *yzb="{\"data\":{\"wordsInfo\":[\"云智泊无人值守停车场\",\"本地时间\",\"减速慢行\",\"一车一杆\",\"关\"],\"restart_time\":0,\"isCheckOpen\":0,\"issense\":1,\"is_special_park\":2,\"type\":2,\"set_broadcast_btime\":\"05:02:02-20:05:05\",\"set_broadcast_bvolume\":5,\"set_broadcast_wtim\":\"undefined-undefined\",\"set_broadcast_wvolume\":4,\"qrcodeUrl\":\"http://bate.cloud-ipark.com:8087/#/bingchannel?id=yzb00023\",\"freeAccessForOffline\":true,\"isCashopen\":2,\"isQrcode\":1,\"licType\":0,\"controlCardType\":2,\"screenType\":0,\"screenRow\":1,\"delayedOffTime\":2,\"monitorIp\":\"1.1.1.1\",\"lightConfig\":[{\"start\":\"00:00\",\"end\":\"07:30\",\"index\":\"1\",\"light\":\"10\"},{\"start\":\"07:30\",\"end\":\"20:00\",\"index\":\"2\",\"light\":\"100\"},{\"start\":\"20:00\",\"end\":\"24:00\",\"index\":\"3\",\"light\":\"100\"}],\"breakPsw\":\"0001\"},\"totalPage\":0,\"total\":1,\"success\":true,\"rtnCode\":200,\"rtnMsg\":\"\",\"url\":\"devInit\"}";

int main()
{
	
	
	pooList_t list = create_pool_list();
	pool_t pool = get_pool(list,4096);
	json_initHooks(pool);
	json_st *j = json_parse(yzb);
	
	
	if(!j) printf("null cjson\r\n");
	printf("size:%d\r\n",json_getArraySize(j));
	json_st *my_j = json_getObjectItem(json_getArrayItem(j,0),"is_special_park");
	
	char *j1 = json_print(my_j);
	printf("type:%d %s %d\r\n",my_j->type,j1,atoi(j1));
	printf("%s\r\n",json_print(j));
	
	//json_delete(j);
	
	j = json_createObject();
	json_st *gg = json_createTrue();
	json_addItemToObject(j,"hhhh",gg);
	
	printf("%s\r\n",json_print(j));
	 
	//json_delete(j);
	
	printf_num();
	destroy_pool(pool);
	free_pool_list(list);
	return 0;
}

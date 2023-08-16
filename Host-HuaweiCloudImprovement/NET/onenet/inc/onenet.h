#ifndef _ONENET_H_
#define _ONENET_H_


#define SET_TOPIC  		"$oc/devices/product_device/sys/messages/down"  		 // ??: ?????????
#define POST_TOPIC 		"$oc/devices/product_device/sys/properties/report"  // ??:??????
#define message_TOPIC	"$oc/devices/product_device/sys/messages/up" 		 // ????



_Bool OneNet_DevLink(void);

void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt);

void OneNet_Publish(const char *topic, const char *msg);

void OneNet_RevPro(unsigned char *cmd);



#endif

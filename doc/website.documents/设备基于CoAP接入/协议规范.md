### 1. 协议版本：
支持 RFC 7252 Constrained Application Protocol协议，具体请参考：[RFC 7252](http://tools.ietf.org/html/rfc7252)
### 2. 通道安全
使用 DTLS v1.2保证通道安全，具体请参考：[DTLS v1.2](https://tools.ietf.org/html/rfc6347)
### 3. 开源客户端参考
http://coap.technology/impls.html
(若使用第三方代码, 阿里云不提供技术支持)

### 4. 阿里CoAP约定
- 不支持？号形式传参数
- 暂时不支持资源发现
- 仅支持UDP协议，并且目前必须通过DTLS
- URI规范, CoAP的URI资源和MQTT TOPIC保持一致，参考[MQTT规范](~~30540~~)


// index.js
const app = getApp() 

const{ connect } = require('../../utils/mqtt')

const mqttHost = 'broker.emqx.io'
const mqttPort = 8084

const deviceSubTopic = '/mysmarthomeriverchen/sub'
const devicePubTopic = '/mysmarthomeriverchen/pub'

const mpSubTopic = devicePubTopic
const mpPubTopic = deviceSubTopic

Page({
  data: {
    client:null,
    Temp:0,
    Hum:0,
    Light:0,
    Led:false,
    Beep:false
  },
  onLedChange(event){
    const that = this
    console.log(event.detail.value);
    const sw = event.detail.value
    that.setData({Led:sw})

    if(sw){
      that.data.client.publish(mpPubTopic,JSON.stringify({
        target:"LED",
        value:1
      }),function(err){
        if(!err){
          console.log('成功下发指令————开灯');
        }
      })
    }else{
      that.data.client.publish(mpPubTopic,JSON.stringify({
        target:"LED",
        value:0
      }),function(err){
        if(!err){
          console.log('成功下发指令————关灯');
        }
      })  
    }
  },
  onBeepChange(event){
    const that = this
    console.log(event.detail.value);
    const sw = event.detail.value
    that.setData({Led:sw})

    if(sw){
      that.data.client.publish(mpPubTopic,JSON.stringify({
        target:"BEEP",
        value:1
      }),function(err){
        if(!err){
          console.log('成功下发指令————打开报警器');
        }
      })
    }else{
      that.data.client.publish(mpPubTopic,JSON.stringify({
        target:"Beep",
        value:0
      }),function(err){
        if(!err){
          console.log('成功下发指令————关闭报警器');
        }
      })  
    }
  },
  onShow(){
      const that = this
      that.setData({
        client:connect(`wxs://${mqttHost}:${mqttPort}/mqtt`)
      })

      that.data.client.on('connect',function(){
        console.log('成功连接到MQTT服务器！');
        wx.showToast({
          title: '连接成功',
          icon:'success',
          mask:true
        })
        that.data.client.subscribe(mpSubTopic,function(err){
            if(!err){
              console.log('成功订阅设备上行数据Topic！');
            }
        })
      })

      that.data.client.on('message',function(topic,message){
        console.log(topic);

        let dataFromDev ={}
        try {
          dataFromDev = JSON.parse(message)
          console.log(dataFromDev);
          that.setData({
            Temp:dataFromDev.Temp,
            Hum:dataFromDev.Hum,
            Light:dataFromDev.Light,
            Led:dataFromDev.Led,
            Beep:dataFromDev.Beep,
          })

        } catch (error) {
          console.log('JSON解析失败',error);
        }
      })
  }
})

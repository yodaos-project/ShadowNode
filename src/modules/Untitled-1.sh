#!/bin/bash
times=0
while :
do
  tts="为你播放${times}"
  messageId="b17f23008acb401095c59535866${times}"
  echo "@@    $tts ==== $messageId     @@"
  curl -H "Content-Type:application/octet-stream" -H "qos:1" -H "retain:false" -H "Topic:u/C3EA6B6FC75A45978CB77555FE771124/rc" -X POST -d '{"reviceDevice":{"accountId":"C3EA6B6FC75A45978CB77555FE771124"},"sourceDevice":{"accountId":"XYFH3311866000C1"},"topic":"card","text":"{\"type\":\"Summary\",\"template\":\"{\\\"type\\\":\\\"Image\\\",\\\"title\\\":\\\"故事\\\",\\\"subtitle\\\":\\\"\\\",\\\"icon\\\":\\\"https://s.rokidcdn.com/mobile-app/icon/card/music.png\\\",\\\"items\\\":[{\\\"title\\\":\\\"【钱儿爸】闭门思过\\\",\\\"subtitle\\\":\\\"Michael钱儿成语启蒙\\\",\\\"imageUrl\\\":\\\"http://imagev2.xmcdn.com/group25/M01/A3/7F/wKgJNlir96_wSYlrAAEsoZmT3ug140.jpg!op_type\\\\u003d3\\\\u0026columns\\\\u003d640\\\\u0026rows\\\\u003d640\\\",\\\"imageType\\\":\\\"Circle\\\",\\\"linkUrl\\\":\\\"rokid://media/v3/detail?id\\\\u003d12316396\\\\u0026appId\\\\u003dR165ECD08C90491B89C809753D1F322F\\\\u0026style\\\\u003ddefault\\\",\\\"contents\\\":[null]}],\\\"buttons\\\":[{\\\"title\\\":\\\"查看专辑详情\\\",\\\"url\\\":\\\"rokid://media/v3/detail?id\\\\u003d12316396\\\\u0026appId\\\\u003dR165ECD08C90491B89C809753D1F322F\\\\u0026style\\\\u003ddefault\\\"}],\\\"extend\\\":\\\"{\\\\\\\"isBuy\\\\\\\":false,\\\\\\\"tts\\\\\\\":\\\\\\\"'${tts}'\\\\\\\",\\\\\\\"coverUrl\\\\\\\":\\\\\\\"http://fdfs.xmcdn.com/group36/M07/2C/FE/wKgJUlpF5QSA7VFlAAEaympF3s0899_mobile_large.jpg\\\\\\\",\\\\\\\"title\\\\\\\":\\\\\\\">【钱儿爸】闭门思过\\\\\\\",\\\\\\\"announcer\\\\\\\":\\\\\\\"Michael钱儿频道\\\\\\\",\\\\\\\"albumTitle\\\\\\\":\\\\\\\"Michael钱儿成语启蒙\\\\\\\",\\\\\\\"seq\\\\\\\":53,\\\\\\\"trackCount\\\\\\\":54}\\\"}\",\"appid\":\"R165ECD08C90491B89C809753D1F322F\",\"feedback\":{\"voice\":\"换一个\"},\"tts\":\"为你播放1\"}","messageId":"'${messageId}'"}' 127.0.0.1:18884/api/sys/broadcastMessage
  times=$((times+1))
  sleep 5
done
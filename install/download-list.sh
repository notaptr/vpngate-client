#!/bin/bash


#
# Загрузка списка серверов
# 130.158.75.35 - vpngate.net
# 130.158.75.40 - https

NAME="vpngate_"`date +%Y-%m-%d`".csv"
wget -nd -O ${NAME} https://www.vpngate.net/api/iphone/


# T31 CSV 字段偏移量映射表

| 偏移量 | 字段名 | CSV列名 | 说明 |
|--------|--------|---------|------|
| off_8_c80 | char[80] | tradingDay, instrumentId, actionDay | 字符串 |
| off_88_d3[0] | double[3] | lastPrice | 最新价 |
| off_88_d3[1] | double[3] | highPrice | 最高价 |
| off_88_d3[2] | double[3] | lowPrice | 最低价 |
| off_112_i2[0] | int32[2] | lastVolume | 最近成交量 |
| off_112_i2[1] | int32[2] | volume | 成交量 |
| off_120_d1 | double | turnover | 成交额 |
| off_128_i3[0] | int32[3] | preOpenInterest | 昨持仓 |
| off_128_i3[1] | int32[3] | openInterest | 持仓量 |
| off_140_d8[0] | double[8] | settlementPrice | 结算价 |
| off_140_d8[3] | double[8] | upperLimitPrice | 涨停价 |
| off_140_d8[4] | double[8] | lowerLimitPrice | 跌停价 |
| off_140_d8[5] | double[8] | preSettlementPrice | 前结算价 |
| off_140_d8[6] | double[8] | preClosePrice | 前收盘价 |
| off_140_d8[7] | double[8] | bidPrice | 买价 |
| off_204_i2[0] | int32[2] | bidQty | 买量 |
| off_204_i2[1] | int32[2] | bidImplyQty | 买推导量 |
| off_212_d1 | double | askPrice | 卖价 |
| off_220_i2[0] | int32[2] | askQty | 卖量 |
| off_220_i2[1] | int32[2] | askImplyQty | 卖推导量 |
| off_228_d1 | double | avgPrice | 均价 |
| off_252_d2[0] | double[2] | openPrice | 开盘价 |
| off_252_d2[1] | double[2] | closePrice | 收盘价 |
| off_236_c16 | char[16] | updateTime, updateMillisec | 时间 |
| off_268_i1 | int32 | priceNum | 价位数量 |
| off_272_blocks | Block32[10] | price1~10, value1~10A,value1~10B, direct1~10 | 10档数据 |

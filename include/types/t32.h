#pragma pack(push, 1)
struct Quot
	{
		double         OrderPrice;                           /*价格*/
		int32_t        OrderQty;                             /*委托量*/
		int32_t        ImplyQty;                             /*推导量*/
		char           BsFlag;                               /*买卖标志*///'1'买,'3'卖
		char           GenTime[13];                          /*生成时间*/
		char           FP[2];                                //pack
	};


struct T32{
		char           type;                                 //行情域标识'2'
		char           FP[3];                                //pack4
		uint32_t       length;                               //报文长度 532
		char           tradingDay[9];                         /*交易日期*/
		char           instrumentId[83];                   /*套利合约号*/
		uint32_t       tid;                                  /*事务编号*/
		double         lastPrice;                            /*最新价*/
		double         LowPrice;                             /*最低价*/
		double         HighPrice;                            /*最高价*/
		double         LifeLow;                              /*历史最低价*/
		double         LifeHigh;                             /*历史最高价*/
		double         RiseLimit;                            /*涨停板*/
		double         FallLimit;                            /*跌停板*/
		double         BidPrice;                             /*最高买*/
		uint32_t       BidQty;                               /*申买量*/
		double         AskPrice;                             /*最低卖*/
		uint32_t       AskQty;                               /*申卖量*/
		char           GenTime[9];                           /*生成时间*/
		char           FP2[3];                               //pack
		uint32_t       MBLCount;                             //价位数量
		Quot   Bid[5];
		Quot   Ask[5];
		char           Unknown[4];
		uint32_t       SequenceNo;                           //序号
		uint32_t       EndCode;                              //0d0a0000
	};//结构体大小为532


    #pragma pack(pop)

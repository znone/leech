let s ={
	STATUS: {
	    // copy from order system
		ODD_PAYSTATUS : {
			ODD_PAYSTATUS_WAIT: 100, //待支付
			ODD_PAYSTATUS_NOT        : 101, //未支付
			ODD_PAYSTATUS_ING        : 102, //支付中
			ODD_PAYSTATUS_FAILD      : 103, //支付失败
			ODD_PAYSTATUS_SUCC       : 104, //支付成功
	},

		ODD_SETTLEMENTSTATUS : {
			ODD_SETTLEMENTSTATUS_NOT: 201, //未结算
			ODD_SETTLEMENTSTATUS_ING      : 202, //结算中
			ODD_SETTLEMENTSTATUS_COM      : 203, //已结算
			ODD_SETTLEMENTSTATUS_AGIN     : 204, //二次结算
		}
	}
};

function isNeedPay(payinfo) {

	console.log(1, 1 in s.STATUS.ODD_PAYSTATUS);
	console.log(101, 101 in s.STATUS.ODD_PAYSTATUS);
	console.log(104, 104 in s.STATUS.ODD_PAYSTATUS);
	console.log("ODD_PAYSTATUS_NOT", "ODD_PAYSTATUS_NOT" in s.STATUS.ODD_PAYSTATUS);
	console.log("--------------------------");

	//	return payinfo.paystatus != ODD_PAYSTATUS.ODD_PAYSTATUS_ING && payinfo.paystatus != ODD_PAYSTATUS.ODD_PAYSTATUS_SUCC &&
	//		 !(payinfo.paystatus in ODD_SETTLEMENTSTATUS);
}

let payinfo = {
	paystatus: 101
};

isNeedPay(payinfo);


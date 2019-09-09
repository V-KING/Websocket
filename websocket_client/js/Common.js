var Common = {
    /*
         * 计算字串长度(中文算2个字符，其余算1个)
         */
    getStrLen: function (val) {
        var len = 0;
        for (var i = 0; i < val.length; i++) {
            var a = val.charAt(i);
            if (a.match(/[^\x00-\xff]/ig) != null) {
                len += 2;
            }
            else {
                len += 1;
            }
        }
        return len;
    },
    getStrWidth: function (val) {
        var width = 0;
        for (var i = 0; i < val.length; i++) {
            var a = val.charAt(i);
            if (a.match(/[^\x00-\xff]/ig) != null) {
                width += 18;
            }
            else {
                if (!isNaN(a)) {
                    width += 14;
                } else {
                    width += 16;
                }
            }
        }
        return width;
    },
    isIp: function (addr) {
        var reg = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;
        if (reg.exec(addr) != null) {
            if (RegExp.$1 < 0 || RegExp.$1 > 255) return false;
            if (RegExp.$2 < 0 || RegExp.$2 > 255) return false;
            if (RegExp.$3 < 0 || RegExp.$3 > 255) return false;
            if (RegExp.$4 < 0 || RegExp.$4 > 255) return false;
        }
        else {
            return false;
        }
        return true;
    },
    /**
         * 封装Jquery的Ajax
         * @param url
         * @param param 支持Json和字串拼接两种方式
         * @param dataType
         * @param callBack
         */
    jsonAjax: function (url, param, callBack) {
        var ajaxParam = "";
        if (Common.isJson(param)) {
            for (var key in param) {
                ajaxParam += (key + "=" + param[key] + "&");
            }
            ajaxParam = ajaxParam.substr(0, ajaxParam.length - 1);
        } else {
            ajaxParam = param;
        }
        $.ajax({
            type: "post",
            url: url,
            data: ajaxParam,//"foo=bar1&foo=bar2"。
            dataType: "JSON",
            success: callBack,
            error: function (data) {
                //alert("数据交互失败!请联系管理员")
            }
        });
    },
    /*
    同步请求
    */
    jsonAjaxSync: function (url, param, callBack) {
        var ajaxParam = "";
        if (Common.isJson(param)) {
            for (var key in param) {
                ajaxParam += (key + "=" + param[key] + "&");
            }
            ajaxParam = ajaxParam.substr(0, ajaxParam.length - 1);
        } else {
            ajaxParam = param;
        }
        $.ajax({
            type: "post",
            url: url,
            async: false,
            data: ajaxParam,//"foo=bar1&foo=bar2"。
            dataType: "JSON",
            success: callBack,
            error: function (data) {
                //alert("数据交互失败!请联系管理员")
            }
        });
    },
    /**
         * 判断是否Json 是返回true,否则false
         * @param obj
         * @returns {Boolean}
         */
    isJson: function (obj) {
        var isjson = typeof (obj) == "object" && Object.prototype.toString.call(obj).toLowerCase() == "[object object]" && !obj.length
        return isjson;
    },
    getStr: function (val) {
        if (val == "null" || val == null || val == "NULL" || val == undefined) {
            return "";
        } else {
            return val;
        }
    },
    getNowDate: function () {
        var date = new Date();
        var seperator1 = "-";
        var seperator2 = ":";
        var month = date.getMonth() + 1;
        var strDate = date.getDate();
        if (month >= 1 && month <= 9) {
            month = "0" + month;
        }
        if (strDate >= 0 && strDate <= 9) {
            strDate = "0" + strDate;
        }
        var currentdate = date.getFullYear() + seperator1 + month + seperator1 + strDate;
        return currentdate;
    },
    getNowDateTime: function () {
        var date = new Date();
        var seperator1 = "-";
        var seperator2 = ":";
        var month = date.getMonth() + 1;
        var strDate = date.getDate();
        if (month >= 1 && month <= 9) {
            month = "0" + month;
        }
        if (strDate >= 0 && strDate <= 9) {
            strDate = "0" + strDate;
        }
        var currentdate = date.getFullYear() + seperator1 + month + seperator1 + strDate
                + " " + date.getHours() + seperator2 + date.getMinutes()
                + seperator2 + date.getSeconds();
        return currentdate;
    },
    padLeft: function (value) {
        var num = Number(value);
        if (num < 10) {
            return "0" + num;
        }
        return num;
    },
    dateFormatter: function (date) {
        if (!date) {
            return "";
        }
        var time = new Date(parseInt(date.slice(6)));
        var year = time.getFullYear();
        var month = Number(time.getMonth()) + 1;
        var day = time.getDate();
        var v = "";
        if (year != 1) {
            v = year + "-" + Common.padLeft(month) + "-" + Common.padLeft(day);
        }
        return v;
    },
    dateTimeFormatter: function (date) {
        if (!date) {
            return "";
        }
        var time = new Date(parseInt(date.slice(6)));
        var year = time.getFullYear();
        var month = Number(time.getMonth()) + 1;
        var day = time.getDate();
        var hour = time.getHours();
        var minute = time.getMinutes();
        var second = time.getSeconds();
        var v = "";
        if (year != 1) {
            v = year + "-" + Common.padLeft(month) + "-" + Common.padLeft(day) + " " + Common.padLeft(hour) + ":" + Common.padLeft(minute) + ":" + Common.padLeft(second);
        }
        return v;
    }
}

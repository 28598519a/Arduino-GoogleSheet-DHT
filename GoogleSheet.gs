var SpreadSheet = SpreadsheetApp.openById(""); //GS ID
var Sheet;

var Col;
(function(Col) {
    Col[Col["data1"] = 1] = "data1";
    Col[Col["data2"] = 2] = "data2";
    Col[Col["time"] = 3] = "time";
})(Col || (Col = {}));

function doGet(e) {
    var params = e.parameter;
    var d1 = params.ID; //IOT Device IDentity
    var type = params.type; //1為Get
    var ReturnResult;

    //work space=====================
    if (params != "{}" && (e.contentLength != -1 || e.queryString != "")) {
        if (typeof(d1) != "undefined") {
            // "Basename" + DeviceID (d1)
            Sheet = SpreadSheet.getSheetByName("" + d1);
            if (type == 1) {
                //Get
                ReturnResult = searchplace(params);
            } else {
                //Set
                ReturnResult = setting(params);
            }
        } else {
            ReturnResult = "Device not match";
        }
    } else {
        ReturnResult = "Error : 請在請求添加參數"
    }
    return ContentService.createTextOutput(ReturnResult);
}

//=============make json======================
function addjson(object, col, row, num) {
    var res = {};
    var text = JSON.stringify(object);
    if (text === '{}') {
        res = JSON.parse('{"' + num + '":"' + Sheet.getRange(row, col).getValue() + '"}');
    } else {
        res = JSON.parse('{' + text.substring(1, text.length - 1) + ',"' + num + '":"' + Sheet.getRange(row, col).getValue() + '"}');
    }
    return res;
}

function do_linear_search(tag, sheetlen, time) {
    while ((tag <= sheetlen) && (Number(time) < Number(Date.parse(Sheet.getRange(tag, 3).getValue())))) {
        tag++;
        if (tag > sheetlen)
            return -1;
    }
    return tag;
}

function do_binary_search(tag, sheetlen, time) {
    var butt = Number(sheetlen);
    var head = Number(tag);
    var mid = Math.floor((tag + sheetlen) / 2);
    while (butt > head) {
        if (Number(time) < Number(Date.parse(Sheet.getRange(mid, 3).getValue()))) {
            head = mid + 1;
        } else {
            butt = mid - 1;
        }
        mid = Math.floor((head + butt) / 2);
    }
    return head;
}

//Set
function setting(params) {
    var ReturnMessage1;
    var d2 = params.data1; //humidity (dht-22)
    var d3 = params.data2; //temperature (dht-22)
    
    // SET DATA
    if (typeof(d2) != "undefined" || typeof(d3) != "undefined") {
        Sheet.insertRows(2);

        if (typeof(d2) != "undefined") Sheet.getRange(2, Col.data1).setValue(d2);
        if (typeof(d3) != "undefined") Sheet.getRange(2, Col.data2).setValue(d3);
        
        Sheet.getRange(2, Col.time).setValue(Utilities.formatDate(new Date(), "GMT+8", "yyyy/MM/dd HH:mm:ss").toString());

        ReturnMessage1 = "data had set";
    } else {
        ReturnMessage1 = "data not set";
    }

    Sheet.getRange(1, 7).setValue(ReturnMessage1);
    //Sheet.getRange(2, 10).setValue(JSON.stringify(e)); //Debug e
    return ReturnMessage1;
}

//Get
function searchplace(params, json) {
    var searchmode = params.searchmode; //1指定時間；指定行
    var STtime = params.STtime; //起始時間
    var EDtime = params.EDtime; //結尾時間
    var datagetcol = params.datagetcol; //行|
    var datagetrow = params.datagetrow; //列一
    var dataquantity = params.dataquantity; //數量上限
    var json = {
        "info": {},
        "time": {}
    };

    if (dataquantity == "") {
        dataquantity = 5;
    }
    if (searchmode == 1) {
        do_row_search(datagetrow, datagetcol, json);

    } else if (searchmode == 2) {
        Timesearch(STtime, EDtime, json, datagetcol);
    }
    return JSON.stringify(json)
}

//do time search=================
function Timesearch(STtime, EDtime, json, datagetcol) {
    var tnd = Number(STtime); //搜尋時間起點
    var tnd2 = Number(EDtime); //搜尋時間終點
    var tag = 2; //時間起點資料標籤
    var tag2 = 2; //時間起點資料尾標籤
    //var type = Number(searchtype);
    //var sub = [[0,4],[5,7],[8,10],[11,13]];
    var sheetlen = Sheet.getLastRow();

    tag = do_binary_search(tag, sheetlen, tnd); //檢索
    tag2 = do_binary_search(tag2, sheetlen, tnd2); //檢索

    if ((tag != -1)) //若有搜尋到起始時間
    {
        if (tag2 == -1) {
            //若無搜尋到結尾時間則列出剩下資料
            tag2 = sheetlen
        }

        dataquantity = tag2 - tag + 1; //計算DATA數量
        for (var i = 0; i != dataquantity; i++) {
            json.info = addjson(json.info, datagetcol, tag, Number(i + 1));
            json.time = addjson(json.time, 3, tag, Number(i + 1));
            tag++;
        }
    }
}

//do row search==================
function do_row_search(datagetrow, datagetcol, json) {
    var j = Number(datagetrow);
    for (var i = 1; i <= datagetrow; i++) {
        json.info = addjson(json.info, datagetcol, j, i)
        json.time = addjson(json.time, 3, j, i);
        j++;
    }
}

//Debug function=================
function Debug_Set() {
    var e = {
        "parameter": {
            "ID": "TQ0000A",
            "data1": "10",
            "data2": "20"
        }
    }
    var result = doGet(e);
    Logger.log("%s", "Set : " + result.getContent());
}

function Debug_Get() {
    /* 行時間檢索 */
    var e = {
        "parameter": {
            "ID": "TQ0000A",
            "type": "1",
            "searchmode": "2",
            "datagetcol": "1",
            "STtime": "1562671996000",
            "EDtime": "1562671157000"
        }
    }
    var result = doGet(e);
    Logger.log("%s", "Get : " + result.getContent());

    /* 列行檢索 */
    var e = {
        "parameter": {
            "ID": "TQ0000A",
            "type": "1",
            "searchmode": "1",
            "datagetcol": "1",
            "datagetrow": "2",
            "dataquantity": "2"
        }
    }
    var result = doGet(e);
    Logger.log("%s", "Get : " + result.getContent());
}
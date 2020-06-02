
var apiHost={
    search: function (queries, size) {
        if (typeof size=="undefined") size=25;
        var url="api/search";
        var args={queries:JSON.stringify(queries),size:size};
        console.log("GET "+url+"?"+$.param(args));
        return $.get(url,args);
    },
    workload: function () {
        var url="api/workload";
        var args={}
        console.log("GET "+url+"?"+$.param(args));
        return $.get(url,args);
    },
    setting: function () {
        var url="api/setting";
        var args={};
        console.log("GET "+url+"?"+$.param(args));
        return $.get(url,args);
    },
};

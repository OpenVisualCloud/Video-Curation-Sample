$("#query-setting").on("open.zf.reveal", function (e) {
    var panel=$(this);
    panel.find(":not(button)").remove();

    $.each(panel.data('control').params,function (ignored, param) {
        if (param.type=="text") {
            var div=$(".template .input-group[text]").clone();
            div.find("b").text(param.name);
            div.find("input").val(param.value).on("change", function () {
                param.value=$(this).val();
            });
            panel.append(div);
        }
        if (param.type=="number") {
            var div=$(".template .input-group[number]").clone();
            div.find("b").text(param.name);
            div.find("input").val(param.value).on("change", function () {
                param.value=parseFloat($(this).val());
            });
            panel.append(div);
        }
        if (param.type=="list") {
            var div=$(".template .input-group[select]").clone();
            div.find("b").text(param.name);

            var value2id=function (v) {
                return v.replace(" ","-");
            };

            var select=div.find("select");

            var valuemap={};
            $.each(param.values, function (ignored, value) {
                var id=value2id(value);
                valuemap[id]=value;
                select.append('<option value="'+id+'">'+value+'</option>');
            });

            if (Array.isArray(param.value)) { // multiple selections
                select.attr('multiple','multiple');
                var ids=[];
                $.each(param.value, function (ignored, value) {
                    ids.push(value2id(value));
                });
                select.val(ids.length==0?null:ids);
                select.on("change", function () {
                    var values=[];
                    var ids=select.val();
                    $.each(ids==null?[]:ids, function (ignored, value) {
                        values.push(valuemap[value]);
                    });
                    param.value=values;
                });
            } else { // single selection
                select.val(param.value);
                select.on("change", function () {
                    param.value=select.val();
                });
            }
            panel.append(div);
        }
    });
});

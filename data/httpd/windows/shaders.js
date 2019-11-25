function wndShaders(wnd){
    this._pWindow = wnd;
    var srcId = "src_" + (new Date()).getTime();

    this._pWindow.find(".src_code").attr("id",srcId);

    this._pEditor = ace.edit(srcId); 
    this._pEditor.setTheme("ace/theme/monokai");
    this._pEditor.session.setMode("ace/mode/glsl");

    var scope = this;

    this._pWindow.find(".src_files").change(function(){
        if($(this).val().trim().length == 0){
            scope._pEditor.setValue("");
            return;
        }

        scope._fetchShaderSource($(this).val(),function(data){
            scope._pEditor.setValue(data);
            scope._pEditor.clearSelection();
        });
    });
    
    
    this._pWindow.find(".compile").click(function(){
        scope._saveShaderSourceAndUpdate(scope._pWindow.find(".src_files").val(),scope._pEditor.getValue(), function(data){
            
            var content = scope._pWindow.find(".content");
            var errorLog =  scope._pWindow.find(".errorLog");
            
            if(data && data.trim().length>0){
                content.addClass("wError");
                $(".errorMsg").html(data);
                errorLog.show();
            }
            else{
                content.removeClass("wError");
                $(".errorMsg").html("");
                errorLog.hide();
            }
            //scope._fetchShaderErrors(function(data){
            //    var expections = data.split("\n")
            //    console.log(expections);
            //});
        });
    });

    function fetchResources(callback){
        $.ajax({
            url:"/x/data?resources",
            success: function(data) {
               if(callback)callback(data);
            }
        });
    }

    fetchResources(function(data){
        if(!data)return;
        var progs = data["programs"];
        if( (progs = data["programs"]) ){
                for(var i = 0;i<progs.length;i++){
                    scope._pWindow.find(".src_files").append("<option value=\""+progs[i]+"\">"+progs[i]+"</option>");
                }
        }
    });
}

wndShaders.prototype._fetchShaderSource = function(shaderPath,callback){
    $.ajax({
        url:"/x/data?shader_src="+shaderPath,
        contentType:"application/javascript",
        success: function(data) {
           if(callback)callback(data);
        }
    });
}


wndShaders.prototype._fetchShaderErrors = function(callback){
    $.ajax({
        url:"/x/data?shader_errors",
        contentType:"application/javascript",
        success: function(data) {
           if(callback)callback(data);
        }
    });
}

wndShaders.prototype._saveShaderSourceAndUpdate = function(shaderPath,content,callback){
    $.ajax({
        url:"/x/data?update_shader="+shaderPath,
        contentType:"application/javascript",
        method:"POST",
        type:"POST",
        data:content,
        success: function(data) {
           if(callback)callback(data);
        }
    });
}

wndShaders.prototype.resize = function(){

}
window = GUIWindow()
window.dim = glmvec2.new(500,500)
window.color = glmvec4.new(0,0.5,0.5,1)

GUIContext.openWindow(window)

a = GUIColumn()
a:setAlignment(GUIAlign.CENTER);
a:setPadding(20);
a.isAlwaysPacked = true;

fontMat = FontMatLib.getMaterial("res/fonts/andrew.fnt");
 for i=1,2 do 
    local b = GUISpecialTextButton("Aloha "..i,fontMat)
	b.dim = glmvec2.new(150,30)
    a:appendChild(b)
 end
 
 window:appendChild(a)

GUI_SpecialTextButton(a:getChildren()[1]).maxScale = 4
var Body = {
  setColor : function(color){
    $('body').css('color',color);
    // document.querySelector('body').style.color = color;
  },
  setBackGroundColor : function(color){
    $('body').css('backgroundColor',color);
    // document.querySelector('body').style.backgroundColor = color;
  }
}
var h1 = {
  setColor : function(color){
    document.querySelector('h1').style.color = color;
  },
  setBorderColor : function(color){
    document.querySelector('h1').style.borderColor = color;
  }
}
function OtherSetColor(color){
  document.querySelector('.list').style.borderColor = color;
  if(color == 'rgb(102, 103, 171)' ){
    document.querySelector('#intro').style.borderColor = 'rgb(102, 103, 171,0.9)';
  }
  else{
    document.querySelector('#intro').style.borderColor = color;
  }
}
function LinkSetColor(color){
  var alist = document.querySelectorAll('a');
  for(var i = 2; i< alist.length; i++){
    alist[i].style.color = color;
  }
}
function NightDayHandler(self) {
  if(self.value == 'Darkmode'){
    Body.setBackGroundColor('rgb(0,0,0,0.8)');
    Body.setColor('white');
    h1.setColor('white');
    h1.setBorderColor('salmon');
    OtherSetColor('salmon');
    LinkSetColor('salmon');
    self.value = 'Daymode';
  }
  else{
    Body.setBackGroundColor('white');
    Body.setColor('black');
    h1.setColor('darkslategray');
    h1.setBorderColor('rgb(102, 103, 171,0.9)');
    OtherSetColor('rgb(102, 103, 171)');
    LinkSetColor('rgb(102, 103, 171)');
    self.value = 'Darkmode';
  }
}

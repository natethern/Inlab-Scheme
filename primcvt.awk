{
  if(substr($0,1,18) =="PrimitiveProcedure") {
    Par=index($0,"(");
    Col=index($0,",");
    Lcol=index($0,")");
    Name=substr($0,Par+1,Col-Par-1);
    Nargs=substr($0,Col+1,Lcol-Col-1);
    Fname=Name;
    Ename="";
    for(i=1; i<=length(Fname); i++) {
      bst=substr(Fname,i,1);
      if(bst == "?") bst="Q";
      if(bst == "!") bst="X";
      if(bst == "-") bst="_";
      if(bst == "=") bst="E";
      if(bst == ">") bst="G";
      if(bst == "<") bst="L";
      if(bst == "/") bst="S";
      if(bst == "*") bst="T";
      if(bst == "+") bst="P";
      if(bst == "-") bst="M";
      Ename=Ename bst;
    }
    Fname=Ename;
    print "PrimitiveProcedure(P" Fname ",\"" Name "\"," Nargs ")";
  } else {
    print $0;
  }
}

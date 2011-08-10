static double gauss_x[20][20] = {
	{ +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.57735026918963 , +0.57735026918963 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.77459666924148 , +0.00000000000000 , +0.77459666924148 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.86113631159405 , -0.33998104358486 , +0.33998104358486 , +0.86113631159405 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.90617984593866 , -0.53846931010568 , +0.00000000000000 , +0.53846931010568 , +0.90617984593866 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.93246951420315 , -0.66120938646627 , -0.23861918608320 , +0.23861918608320 , +0.66120938646626 , +0.93246951420315 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.94910791234276 , -0.74153118559939 , -0.40584515137740 , +0.00000000000000 , +0.40584515137740 , +0.74153118559939 , +0.94910791234276 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.96028985649754 , -0.79666647741362 , -0.52553240991633 , -0.18343464249565 , +0.18343464249565 , +0.52553240991633 , +0.79666647741362 , +0.96028985649754 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.96816023950762 , -0.83603110732664 , -0.61337143270059 , -0.32425342340381 , +0.00000000000000 , +0.32425342340381 , +0.61337143270059 , +0.83603110732664 , +0.96816023950762 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.97390652851717 , -0.86506336668899 , -0.67940956829903 , -0.43339539412925 , -0.14887433898163 , +0.14887433898163 , +0.43339539412925 , +0.67940956829902 , +0.86506336668899 , +0.97390652851717 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.97822865814606 , -0.88706259976809 , -0.73015200557405 , -0.51909612920681 , -0.26954315595234 , +0.00000000000000 , +0.26954315595235 , +0.51909612920681 , +0.73015200557405 , +0.88706259976810 , +0.97822865814605 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.98156063424670 , -0.90411725637050 , -0.76990267419430 , -0.58731795428662 , -0.36783149899818 , -0.12523340851147 , +0.12523340851147 , +0.36783149899818 , +0.58731795428662 , +0.76990267419430 , +0.90411725637049 , +0.98156063424671 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.98418305471862 , -0.91759839922291 , -0.80157809073333 , -0.64234933944034 , -0.44849275103644 , -0.23045831595514 , +0.00000000000000 , +0.23045831595513 , +0.44849275103644 , +0.64234933944035 , +0.80157809073333 , +0.91759839922293 , +0.98418305471862 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.98628380869676 , -0.92843488366368 , -0.82720131506970 , -0.68729290481170 , -0.51524863635816 , -0.31911236892789 , -0.10805494870734 , +0.10805494870734 , +0.31911236892789 , +0.51524863635816 , +0.68729290481170 , +0.82720131506971 , +0.92843488366365 , +0.98628380869678 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.98799251802062 , -0.93727339240047 , -0.84820658341055 , -0.72441773136014 , -0.57097217260855 , -0.39415134707756 , -0.20119409399743 , +0.00000000000000 , +0.20119409399743 , +0.39415134707756 , +0.57097217260856 , +0.72441773136009 , +0.84820658341065 , +0.93727339240040 , +0.98799251802063 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.98940093499137 , -0.94457502307382 , -0.86563120238740 , -0.75540440835515 , -0.61787624440263 , -0.45801677765722 , -0.28160355077926 , -0.09501250983764 , +0.09501250983764 , +0.28160355077926 , +0.45801677765723 , +0.61787624440260 , +0.75540440835523 , +0.86563120238731 , +0.94457502307384 , +0.98940093499138 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.99057547531396 , -0.95067552176969 , -0.88023915372635 , -0.78151400389700 , -0.65767115921669 , -0.51269053708645 , -0.35123176345388 , -0.17848418149585 , +0.00000000000000 , +0.17848418149585 , +0.35123176345388 , +0.51269053708647 , +0.65767115921662 , +0.78151400389722 , +0.88023915372594 , +0.95067552177008 , +0.99057547531381 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.99156516842123 , -0.95582394957059 , -0.89260246649859 , -0.80370495897161 , -0.69168704306095 , -0.55977083107369 , -0.41175116146290 , -0.25188622569150 , -0.08477501304174 , +0.08477501304174 , +0.25188622569150 , +0.41175116146289 , +0.55977083107379 , +0.69168704306052 , +0.80370495897272 , +0.89260246649689 , +0.95582394957211 , +0.99156516842064 , +0.00000000000000 , +0.00000000000000} , 
	{ -0.99240684384615 , -0.96020815212817 , -0.90315590362245 , -0.82271465653167 , -0.72096617733783 , -0.60054530466089 , -0.46457074137608 , -0.31656409996363 , -0.16035864564023 , +0.00000000000000 , +0.16035864564022 , +0.31656409996363 , +0.46457074137605 , +0.60054530466099 , +0.72096617733769 , +0.82271465653175 , +0.90315590362258 , +0.96020815212794 , +0.99240684384624 , +0.00000000000000} , 
	{ -0.99312859919364 , -0.96397192725716 , -0.91223442827216 , -0.83911697181018 , -0.74633190646409 , -0.63605368072613 , -0.51086700195059 , -0.37370608871551 , -0.22778585114164 , -0.07652652113350 , +0.07652652113350 , +0.22778585114164 , +0.37370608871551 , +0.51086700195055 , +0.63605368072634 , +0.74633190646344 , +0.83911697181146 , +0.91223442827055 , +0.96397192725840 , +0.99312859919322} };
static double gauss_w[20][20] = {
	{ +2.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +1.00000000000000 , +1.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.55555555555556 , +0.88888888888889 , +0.55555555555556 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.34785484513745 , +0.65214515486255 , +0.65214515486255 , +0.34785484513745 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.23692688505619 , +0.47862867049937 , +0.56888888888889 , +0.47862867049937 , +0.23692688505619 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.17132449237918 , +0.36076157304814 , +0.46791393457269 , +0.46791393457269 , +0.36076157304814 , +0.17132449237917 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.12948496616887 , +0.27970539148928 , +0.38183005050512 , +0.41795918367347 , +0.38183005050512 , +0.27970539148928 , +0.12948496616887 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.10122853629037 , +0.22238103445338 , +0.31370664587789 , +0.36268378337836 , +0.36268378337836 , +0.31370664587789 , +0.22238103445338 , +0.10122853629036 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.08127438836158 , +0.18064816069486 , +0.26061069640294 , +0.31234707704000 , +0.33023935500126 , +0.31234707704000 , +0.26061069640293 , +0.18064816069486 , +0.08127438836158 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.06667134430870 , +0.14945134915058 , +0.21908636251598 , +0.26926671931000 , +0.29552422471475 , +0.29552422471475 , +0.26926671931000 , +0.21908636251598 , +0.14945134915059 , +0.06667134430870 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.05566856711617 , +0.12558036946491 , +0.18629021092772 , +0.23319376459199 , +0.26280454451025 , +0.27292508677790 , +0.26280454451025 , +0.23319376459199 , +0.18629021092773 , +0.12558036946490 , +0.05566856711618 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.04717533638657 , +0.10693932599528 , +0.16007832854334 , +0.20316742672307 , +0.23349253653835 , +0.24914704581340 , +0.24914704581340 , +0.23349253653836 , +0.20316742672307 , +0.16007832854334 , +0.10693932599532 , +0.04717533638654 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.04048400476523 , +0.09212149983778 , +0.13887351021975 , +0.17814598076195 , +0.20781604753689 , +0.22628318026290 , +0.23255155323087 , +0.22628318026290 , +0.20781604753689 , +0.17814598076195 , +0.13887351021979 , +0.09212149983776 , +0.04048400476523 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.03511946033188 , +0.08015808715966 , +0.12151857068792 , +0.15720316715817 , +0.18553839747794 , +0.20519846372130 , +0.21526385346316 , +0.21526385346316 , +0.20519846372129 , +0.18553839747793 , +0.15720316715826 , +0.12151857068780 , +0.08015808715974 , +0.03511946033180 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.03075324199574 , +0.07036604748839 , +0.10715922046727 , +0.13957067792617 , +0.16626920581697 , +0.18616100001557 , +0.19843148532711 , +0.20257824192556 , +0.19843148532711 , +0.18616100001556 , +0.16626920581696 , +0.13957067792624 , +0.10715922046719 , +0.07036604748842 , +0.03075324199570 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.02715245941248 , +0.06225352393774 , +0.09515851168284 , +0.12462897125525 , +0.14959598881664 , +0.16915651939500 , +0.18260341504492 , +0.18945061045507 , +0.18945061045507 , +0.18260341504492 , +0.16915651939500 , +0.14959598881663 , +0.12462897125573 , +0.09515851168286 , +0.06225352393813 , +0.02715245941244 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.02414830286977 , +0.05545952937299 , +0.08503614831787 , +0.11188384719300 , +0.13513636846860 , +0.15404576107680 , +0.16800410215645 , +0.17656270536699 , +0.17944647035621 , +0.17656270536699 , +0.16800410215645 , +0.15404576107681 , +0.13513636846845 , +0.11188384719319 , +0.08503614831748 , +0.05545952937176 , +0.02414830287024 , +0.00000000000000 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.02161601352579 , +0.04971454889687 , +0.07642573025339 , +0.10094204410634 , +0.12255520671107 , +0.14064291467064 , +0.15468467512625 , +0.16427648374583 , +0.16914238296314 , +0.16914238296314 , +0.16427648374583 , +0.15468467512627 , +0.14064291467075 , +0.12255520671138 , +0.10094204410861 , +0.07642573025500 , +0.04971454889559 , +0.02161601352740 , +0.00000000000000 , +0.00000000000000} , 
	{ +0.01946178822302 , +0.04481422677517 , +0.06904454273520 , +0.09149002162598 , +0.11156664554753 , +0.12875396253941 , +0.14260670217354 , +0.15276604206586 , +0.15896884339395 , +0.16105444984878 , +0.15896884339395 , +0.15276604206586 , +0.14260670217356 , +0.12875396253946 , +0.11156664554701 , +0.09149002162488 , +0.06904454273209 , +0.04481422677711 , +0.01946178822349 , +0.00000000000000} , 
	{ +0.01761400711764 , +0.04060142981284 , +0.06267204830881 , +0.08327674158126 , +0.10193011981662 , +0.11819453196218 , +0.13168863844906 , +0.14209610931837 , +0.14917298647260 , +0.15275338713073 , +0.15275338713073 , +0.14917298647260 , +0.14209610931838 , +0.13168863844921 , +0.11819453196160 , +0.10193011981930 , +0.08327674157810 , +0.06267204832676 , +0.04060142982424 , +0.01761400711745} };

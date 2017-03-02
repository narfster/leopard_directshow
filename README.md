# leopard_directshow
Directshow c++ support for leopard imagining's UVC cameras.

I was searching around the web, looking for a simple c++ solution on how to work with these UVC cameras over windows. Linux apperently was much simpler and a common solution with examples using v4l2. On windows life is harder, the supporetd SDK from leopard imaging is in c#, it is basicly a wrapper for directshow, and a total mess.  I just needed to grab the frames, and use them for my machine vision framework on c++.  



supports:
- LI-USB30-V034M
- LI-USB30-M021M

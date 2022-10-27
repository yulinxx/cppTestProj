
“状态” 算是 人们对事物一个很基本的抽象理解了，
在现实世界里，“状态” 无时无刻不体现在我们的生活和工作之中；  
现实中客观存在的事物，我们总可以给它定义出几个状态来。  

而在软件领域，也很早就形成了基于状态的行为模型范式，即 有限状态机(Finite-State Machinee，缩写：FSM)。 
又称有限状态自动机（英语：finite-state automation，缩写：FSA），简称状态机，
是表示有限个状态以及在这些状态之间的转移和动作等行为的数学计算模型。

https://cloud.tencent.com/developer/article/2000198

UML建模规范中，专门定义了状态机的行为模型，接下来将介绍几个核心的概念：

- State：状态
 ● Pseudo state , 伪状态，是状态机中的和事物本身没有关系的状态，用以描述状态机完整工作的状态，比如状态机的开始，结束等
 ● Simple State, 真实状态，表示的是真实的事物状态
 ● Composite State, 组合状态，将多个简单状态组合成一个新的状态
 ● Submachine State, 子状态机状态，状态机可以嵌套子状态机，而子状态机可以的整体可以Submachine State 表示。
- Event : 事件，状态流转时锁触发的事件
Transition: 状态流转，表达的是从一个状态到另外一个状态的转换，包含 source state,target state,event 信息
 ● ExternalTransition 两个不同状态之间的流转
 ● Internal Transition 一个状态下的内部流转，不会影响到当前状态改变
- Region 作用域，将某个状态机或者 组合状态
- StateMachine，状态机：以上所有概念的集合。

State-状态
上述例子的图中，所有的节点都称之为状态；而状态会包含如下几种组成部分：
 ❏ state 当前状态的描述
 ❏ entry (进入节点前应当处理的行为)
 ❏ exit (从当前状态退出时应当采取的行为)
 其中，entry、exit 是状态定义的两种行为，而这两种方式就是留给业务拓展的地方。  
 比如待收货状态下，可以在entry行为上，增加通知用户逻辑，而不影响整体状态流转
![](https://ask.qcloudimg.com/http-save/yehe-2562698/c3336810660280313e38c4073ad6052c.png)

 Event-事件
状态内或者不同的状态之间，通过事件的方式进行触发转换。
如下图所示, 从State A 到 State B 发生转换，通过Event 触发。
![](https://ask.qcloudimg.com/http-save/yehe-2562698/401674798bea8657d7b6256ed72b9f4b.png)


 Transition - 状态转换
状态机的状态之间的流转行为，如下图所示，状态和状态之间的连线，在定义上，被称为transition。

![](https://ask.qcloudimg.com/http-save/yehe-2562698/5fc22f1290d3d14a72f79b82d8c4e47e.png)

一个transition 包含五个元素：
 ● source State (原状态): 状态变迁的起始状态
 ● Trigger（触发器）: 触发器是指如何触发transition的形式，可以基于事件，也可以基于定时器。大部分场景下，是基于事件的触发器。根据事件的key 去寻找 transition, 唯一匹配到特定的transition；
 ●target State （目标状态）：事件发生后，应当到达的状态

● guard (门卫)：当事件请求触发时，可以定义校验规则，当满足此规则的时候，则正常执行状态变迁，否则提前终止
 ●actions(动作)：当状态机判断transition 是合法时，会执行 actions。actions是框架层面开放出来的拓展点，请注意，actions 如果执行抛出了异常，则transition 状态变迁将会终止。
 ![](https://ask.qcloudimg.com/http-save/yehe-2562698/bda7706cb263b26b669b49431bc846ff.png)


boost的状态机库(Boost.MetaStateMachine )分为 back-end 和 front-end； 
根据 front-end 的不同有三种实现，而 back-end 只有一种实现。

basic front-end  
function front-end  
eUML front-end  

basic front-end 和 funciton front-end 都是通过创建类，继承类，定义成员变量来实现；  
而 eUML front-end 是通过重用UML状态机，利用一些c++宏来实现的，感觉比较代码清晰简洁
https://blog.csdn.net/weixin_32955207/article/details/112996865
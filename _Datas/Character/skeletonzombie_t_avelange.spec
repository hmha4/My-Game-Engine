<?xml version="1.0" encoding="UTF-8"?>
<GameCharacterSpec xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
    <ModelType>2</ModelType>
    <Name>skeletonzombie_t_avelange</Name>
    <Tag>Enemy</Tag>
    <Colliders>
        <ColliderCount>2</ColliderCount>
        <Name>Main Collider Enemy</Name>
        <Type>1</Type>
        <BoneIndex>-1</BoneIndex>
        <RootMatrix>
            <Position>
                <X>0</X>
                <Y>1</Y>
                <Z>0</Z>
            </Position>
            <Rotation>
                <X>0</X>
                <Y>0</Y>
                <Z>0</Z>
            </Rotation>
            <Scale>
                <X>1</X>
                <Y>1</Y>
                <Z>1</Z>
            </Scale>
        </RootMatrix>
        <Name>RightHand Collider</Name>
        <Type>1</Type>
        <BoneIndex>34</BoneIndex>
        <RootMatrix>
            <Position>
                <X>-22.299999</X>
                <Y>0</Y>
                <Z>0</Z>
            </Position>
            <Rotation>
                <X>0</X>
                <Y>0</Y>
                <Z>0</Z>
            </Rotation>
            <Scale>
                <X>20</X>
                <Y>20</Y>
                <Z>20</Z>
            </Scale>
        </RootMatrix>
    </Colliders>
  <Trails>
    <TrailCount>0</TrailCount>
  </Trails>
    <IsChild>0</IsChild>
    <ParentBoneIndex>-1</ParentBoneIndex>
    <ParentName></ParentName>
    <Position>
        <X>0</X>
        <Y>0</Y>
        <Z>0</Z>
    </Position>
    <Rotation>
        <X>0</X>
        <Y>0</Y>
        <Z>0</Z>
    </Rotation>
    <Scale>
        <X>1</X>
        <Y>1</Y>
        <Z>1</Z>
    </Scale>
    <ClipNames>
        <ClipCount>8</ClipCount>
        <ClipName>Idle</ClipName>
        <ClipName>Walk</ClipName>
        <ClipName>Run</ClipName>
        <ClipName>Seek</ClipName>
        <ClipName>Alert</ClipName>
        <ClipName>Attack</ClipName>
        <ClipName>Die</ClipName>
        <ClipName>Hit</ClipName>
    </ClipNames>
    <FilePath>skeletonzombie_t_avelange</FilePath>
    <BTName>Enemy</BTName>
    <BehaviourTree>
        <Decorator>Root</Decorator>
        <ChildCount>1</ChildCount>
        <Child>
            <Type>Selector</Type>
            <Name>Selector</Name>
        </Child>
        <Selector>Selector</Selector>
        <ChildCount>4</ChildCount>
        <Child>
            <Type>Sequence</Type>
            <Name>Sequence</Name>
            <Type>Sequence</Type>
            <Name>Sequence</Name>
            <Type>Sequence</Type>
            <Name>Sequence</Name>
            <Type>Selector</Type>
            <Name>Selector</Name>
        </Child>
        <Sequence>Sequence</Sequence>
        <ChildCount>2</ChildCount>
        <Child>
            <Type>Condition</Type>
            <Name>NumericLifeUnder</Name>
            <Value>1</Value>
            <CompareType>0</CompareType>
            <Type>Task</Type>
            <Name>ActionDie</Name>
        </Child>
        <Condition>NumericLifeUnder</Condition>
        <ChildCount>0</ChildCount>
        <Child/>
        <Task>ActionDie</Task>
        <ChildCount>0</ChildCount>
        <Child/>
        <Sequence>Sequence</Sequence>
        <ChildCount>2</ChildCount>
        <Child>
            <Type>Condition</Type>
            <Name>FunctionCollision</Name>
            <Check>1</Check>
            <Type>Task</Type>
            <Name>ActionHit</Name>
        </Child>
        <Condition>FunctionCollision</Condition>
        <ChildCount>0</ChildCount>
        <Child/>
        <Task>ActionHit</Task>
        <ChildCount>0</ChildCount>
        <Child/>
        <Sequence>Sequence</Sequence>
        <ChildCount>2</ChildCount>
        <Child>
            <Type>Condition</Type>
            <Name>NumericAlertDistance</Name>
            <Value>30</Value>
            <CompareType>0</CompareType>
            <Type>Selector</Type>
            <Name>Selector</Name>
        </Child>
        <Condition>NumericAlertDistance</Condition>
        <ChildCount>0</ChildCount>
        <Child/>
        <Selector>Selector</Selector>
        <ChildCount>3</ChildCount>
        <Child>
            <Type>Task</Type>
            <Name>ActionScreamNTurn</Name>
            <Type>Sequence</Type>
            <Name>Sequence</Name>
            <Type>Task</Type>
            <Name>ActionRun</Name>
        </Child>
        <Task>ActionScreamNTurn</Task>
        <ChildCount>0</ChildCount>
        <Child/>
        <Sequence>Sequence</Sequence>
        <ChildCount>2</ChildCount>
        <Child>
            <Type>Condition</Type>
            <Name>NumericAttackDistance</Name>
            <Value>5</Value>
            <CompareType>0</CompareType>
            <Type>Task</Type>
            <Name>ActionAttack</Name>
        </Child>
        <Condition>NumericAttackDistance</Condition>
        <ChildCount>0</ChildCount>
        <Child/>
        <Task>ActionAttack</Task>
        <ChildCount>0</ChildCount>
        <Child/>
        <Task>ActionRun</Task>
        <ChildCount>0</ChildCount>
        <Child/>
        <Selector>Selector</Selector>
        <ChildCount>3</ChildCount>
        <Child>
            <Type>Task</Type>
            <Name>ActionTurn</Name>
            <Type>Task</Type>
            <Name>ActionMove</Name>
            <Type>Sequence</Type>
            <Name>Sequence</Name>
        </Child>
        <Task>ActionTurn</Task>
        <ChildCount>0</ChildCount>
        <Child/>
        <Task>ActionMove</Task>
        <ChildCount>0</ChildCount>
        <Child/>
        <Sequence>Sequence</Sequence>
        <ChildCount>3</ChildCount>
        <Child>
            <Type>Task</Type>
            <Name>ActionIdle</Name>
            <Type>Condition</Type>
            <Name>NumericRandom</Name>
            <Value>1</Value>
            <CompareType>2</CompareType>
            <Type>Task</Type>
            <Name>ActionSeek</Name>
        </Child>
        <Task>ActionIdle</Task>
        <ChildCount>0</ChildCount>
        <Child/>
        <Condition>NumericRandom</Condition>
        <ChildCount>0</ChildCount>
        <Child/>
        <Task>ActionSeek</Task>
        <ChildCount>0</ChildCount>
        <Child/>
    </BehaviourTree>
</GameCharacterSpec>
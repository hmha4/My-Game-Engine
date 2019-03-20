<?xml version="1.0" encoding="UTF-8"?>
<GameCharacterSpec xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
    <ModelType>2</ModelType>
    <Name>Brute</Name>
    <Tag></Tag>
    <Colliders>
        <ColliderCount>0</ColliderCount>
    </Colliders>
    <IsChild>false</IsChild>
    <ParentBoneIndex>-1</ParentBoneIndex>
    <ClipNames>
        <ClipCount>0</ClipCount>
    </ClipNames>
    <FilePath>Brute</FilePath>
    <BTName>Player 0</BTName>
    <BehaviourTree>
        <Decorator>Root</Decorator>
        <ChildCount>1</ChildCount>
        <Child>
            <Sequence>Sequence</Sequence>
        </Child>
        <Sequence>Sequence</Sequence>
        <ChildCount>2</ChildCount>
        <Child>
            <Task>Task</Task>
            <Task>Task123</Task>
        </Child>
        <Task>Task</Task>
        <ChildCount/>
        <Child/>
        <Task>Task123</Task>
        <ChildCount/>
        <Child/>
    </BehaviourTree>
</GameCharacterSpec>

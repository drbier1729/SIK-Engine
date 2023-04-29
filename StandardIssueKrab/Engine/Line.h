#pragma once

class Line 
{
public:
    Line() = default;
    ~Line() = default;


    Uint64 vaoID;
    std::vector<float> vertices;



    virtual void DrawLine() const = 0;
};

class Arrow : Line {
public:
    Arrow();
    void DrawLine() const;
};
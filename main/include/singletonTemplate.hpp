#pragma once

template<typename T>
class Singleton{
protected:
    Singleton(){} // Disallow instantiation outside of the class.
public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton &) = delete;
    Singleton(Singleton &&) = delete;
    Singleton & operator=(Singleton &&) = delete;

    static T& instance(){
        static T instance;
        return instance;
    }
};
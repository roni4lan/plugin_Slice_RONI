#include "Polyline.h"
#include <cmath>
#include <stdexcept>
namespace sb {
float Polyline::length(const std::vector<glm::vec2>& p,bool closed) {
    float result=0; if(p.size()<2) return 0;
    for(size_t i=1;i<p.size();++i) result+=glm::distance(p[i-1],p[i]);
    return result+(closed?glm::distance(p.back(),p.front()):0);
}
float Polyline::area(const std::vector<glm::vec2>& p) {
    double sum=0; for(size_t i=0;i<p.size();++i) { auto a=p[i],b=p[(i+1)%p.size()]; sum+=double(a.x)*b.y-double(b.x)*a.y; }
    return sum/2;
}
std::vector<glm::vec2> Polyline::resample(const std::vector<glm::vec2>& p,float spacing,bool closed) {
    float total=length(p,closed); if(total<0.001f || spacing<=0) throw std::runtime_error("Contorno degenerado");
    int segments=std::max(closed?3:1,int(std::ceil(total/spacing)));
    std::vector<glm::vec2> result; size_t edge=0; float start=0;
    for(int i=0;i<(closed?segments:segments+1);++i) {
        float distance=total*i/segments;
        while(edge+1<(closed?p.size():p.size()-1) && start+glm::distance(p[edge],p[(edge+1)%p.size()])<distance) {
            start+=glm::distance(p[edge],p[(edge+1)%p.size()]); ++edge;
        }
        auto a=p[edge],b=p[(edge+1)%p.size()]; float len=glm::distance(a,b);
        result.push_back(glm::mix(a,b,len>0?(distance-start)/len:0));
    }
    return result;
}
std::vector<glm::vec2> Polyline::rounded(const std::vector<glm::vec2>& p,float radius,int subdivision,bool closed) {
    if(radius<=0 || p.size()<3) return p;
    std::vector<glm::vec2> result;
    for(size_t i=0;i<p.size();++i) {
        if(!closed && (i==0 || i+1==p.size())) { result.push_back(p[i]); continue; }
        auto a=p[(i+p.size()-1)%p.size()]-p[i],b=p[(i+1)%p.size()]-p[i];
        float la=glm::length(a),lb=glm::length(b);
        if(la<0.001f || lb<0.001f) throw std::runtime_error("Puntos duplicados");
        a/=la; b/=lb; float angle=std::acos(ofClamp(glm::dot(a,b),-1.f,1.f));
        if(angle<0.001f || angle>PI-0.001f) { result.push_back(p[i]); continue; }
        float tangent=std::min(radius/std::tan(angle/2),0.49f*std::min(la,lb));
        auto center=p[i]+glm::normalize(a+b)*(tangent/std::cos(angle/2));
        auto start=p[i]+a*tangent, end=p[i]+b*tangent;
        float theta=std::atan2(start.y-center.y,start.x-center.x);
        float delta=std::atan2(end.y-center.y,end.x-center.x)-theta;
        while(delta>PI) delta-=TWO_PI; while(delta<-PI) delta+=TWO_PI;
        float r=glm::distance(start,center);
        for(int j=0;j<=subdivision;++j) { float t=theta+delta*j/subdivision; result.push_back(center+glm::vec2(std::cos(t),std::sin(t))*r); }
    }
    return result;
}
std::vector<glm::vec2> Polyline::offset(const std::vector<glm::vec2>& p,float amount,float limit,bool closed) {
    if(std::abs(amount)<0.0001f) return p;
    auto result=p; float winding=area(p)>=0?1.f:-1.f;
    for(size_t i=0;i<p.size();++i) {
        auto prev=p[i]-p[(i+p.size()-1)%p.size()],next=p[(i+1)%p.size()]-p[i];
        if(!closed && i==0) prev=next;
        if(!closed && i+1==p.size()) next=prev;
        if(glm::length(prev)<0.001f || glm::length(next)<0.001f) throw std::runtime_error("Contorno degenerado");
        prev=glm::normalize(prev); next=glm::normalize(next);
        glm::vec2 n1{-prev.y*winding,prev.x*winding},n2{-next.y*winding,next.x*winding};
        auto sum=n1+n2;
        if(glm::length(sum)<0.001f) throw std::runtime_error("Contorno con giro de 180 grados");
        auto m=glm::normalize(sum); float scale=1/std::max(0.001f,glm::dot(m,n1));
        result[i]+=m*amount*std::min(scale,limit);
    }
    if(closed && area(result)*area(p)<=0) throw std::runtime_error("Offset colapsa el contorno");
    for(size_t i=0;i<(closed?p.size():p.size()-1);++i) {
        size_t j=(i+1)%p.size();
        if(glm::dot(result[j]-result[i],p[j]-p[i])<=0)
            throw std::runtime_error("Offset invierte una arista; reducir offset/radio");
    }
    return result;
}
}

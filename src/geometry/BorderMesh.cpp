#include "BorderMesh.h"
#include "Polyline.h"
#include <cmath>
namespace sb {
BorderMesh BorderMesh::build(const std::vector<glm::vec2>& source,const ofJson& p) {
    BorderMesh out; bool closed=p.at("closePath").get<bool>();
    float thickness=p.at("thickness"),half=thickness/2;
    std::string alignment=p.at("alignment"),join=p.at("joinStyle"),cap=p.at("capStyle"),mode=p.at("segmentsMode");
    float center=p.at("offset").get<float>()+(alignment=="inside"?half:alignment=="outside"?-half:0);
    float limit=p.at("miterLimit"); int subdivisions=p.at("subdivision"),mask=p.at("edgeMask");
    out.contour=Polyline::offset(Polyline::rounded(source,p.at("cornerRadius"),subdivisions,closed),center,limit,closed);
    auto& pts=out.contour; size_t n=pts.size(); if(n<2) throw std::runtime_error("Slice sin contorno");
    out.perimeter=Polyline::length(pts,closed); out.mesh.setMode(OF_PRIMITIVE_TRIANGLES);
    float winding=Polyline::area(pts)>=0?1.f:-1.f;
    std::vector<glm::vec2> directions(n),normals(n),miters(n);
    std::vector<float> u(n+1,0),ratios(n,1);
    for(size_t i=0;i<(closed?n:n-1);++i) {
        auto d=pts[(i+1)%n]-pts[i]; if(glm::length(d)<0.001f) throw std::runtime_error("Arista degenerada");
        directions[i]=glm::normalize(d); normals[i]={-directions[i].y*winding,directions[i].x*winding};
        u[i+1]=u[i]+glm::length(d);
    }
    if(!closed) { directions[n-1]=directions[n-2]; normals[n-1]=normals[n-2]; }
    for(size_t i=0;i<n;++i) {
        auto n1=(!closed && i==0)?normals[i]:normals[(i+n-1)%n];
        auto n2=normals[i];
        if(glm::length(n1+n2)<0.001f) throw std::runtime_error("Giro de 180 grados no soportado");
        miters[i]=glm::normalize(n1+n2); ratios[i]=1/std::max(0.001f,glm::dot(miters[i],n2));
    }
    auto emit=[&](glm::vec2 pt,float distance,float v){out.mesh.addVertex({pt.x,pt.y,0});out.mesh.addTexCoord({distance,v});};
    auto triangle=[&](glm::vec2 a,glm::vec2 b,glm::vec2 c,float distance,float av,float bv,float cv){emit(a,distance,av);emit(b,distance,bv);emit(c,distance,cv);};
    auto active=[&](size_t edge){
        if(mode!="selectedEdges") return true;
        // Rounded corners add vertices: assign each new edge to the closest original edge.
        auto middle=(pts[edge]+pts[(edge+1)%n])/2.f; size_t best=0; float bestDistance=std::numeric_limits<float>::max();
        for(size_t k=0;k<(closed?source.size():source.size()-1);++k){auto a=source[k],d=source[(k+1)%source.size()]-a;
            float t=ofClamp(glm::dot(middle-a,d)/glm::dot(d,d),0.f,1.f);float distance=glm::distance(middle,a+d*t);
            if(distance<bestDistance){bestDistance=distance;best=k;}}
        return (mask&(1<<best))!=0;
    };
    auto turn=[&](size_t i){auto a=directions[(i+n-1)%n],b=directions[i];return (a.x*b.y-a.y*b.x)*winding;};
    auto endpoint=[&](size_t vertex,size_t edge,float side){
        if((!closed && (vertex==0 || vertex==n-1)) || mode=="perEdge") return pts[vertex]+normals[edge]*half*side;
        float external=turn(vertex)>=0?-1.f:1.f;
        bool useMiter=(join=="miter" && ratios[vertex]<=limit) || side!=external;
        return pts[vertex]+(useMiter?miters[vertex]*std::min(ratios[vertex],limit):normals[edge])*half*side;
    };
    for(size_t i=0;i<(closed?n:n-1);++i) {
        if(!active(i)) continue; size_t j=(i+1)%n;
        auto a=endpoint(i,i,-1),b=endpoint(i,i,1),c=endpoint(j,i,-1),d=endpoint(j,i,1);
        bool startCap=mode=="perEdge" || (!closed && i==0) || !active((i+n-1)%n);
        bool endCap=mode=="perEdge" || (!closed && j==n-1) || !active(j);
        if(cap=="square") { if(startCap){a-=directions[i]*half;b-=directions[i]*half;} if(endCap){c+=directions[i]*half;d+=directions[i]*half;} }
        emit(a,u[i],-1);emit(b,u[i],1);emit(c,u[i+1],-1);
        emit(b,u[i],1);emit(d,u[i+1],1);emit(c,u[i+1],-1);
        if(cap=="round") for(int end=0;end<2;++end) {
            if(!(end?endCap:startCap)) continue; auto origin=pts[end?j:i]; float dist=end?u[i+1]:u[i];
            float theta=std::atan2(normals[i].y,normals[i].x); float sign=end?-winding:winding;
            for(int k=0;k<subdivisions;++k) {
                float t1=theta+sign*PI*k/subdivisions,t2=theta+sign*PI*(k+1)/subdivisions;
                auto p1=glm::vec2(std::cos(t1),std::sin(t1)),p2=glm::vec2(std::cos(t2),std::sin(t2));
                triangle(origin,origin+p1*half,origin+p2*half,dist,0,glm::dot(p1,normals[i]),glm::dot(p2,normals[i]));
            }
        }
        if(mode=="perEdge" || endCap || std::abs(turn(j))<0.0001f || (join=="miter" && ratios[j]<=limit)) continue;
        float side=turn(j)>=0?-1.f:1.f;
        auto anchor=endpoint(j,i,-side),from=normals[i]*side,to=normals[j]*side;
        if(join=="round") {
            float theta=std::atan2(from.y,from.x),delta=std::atan2(to.y,to.x)-theta;
            while(delta>PI)delta-=TWO_PI;while(delta<-PI)delta+=TWO_PI;
            for(int k=0;k<subdivisions;++k){float t1=theta+delta*k/subdivisions,t2=theta+delta*(k+1)/subdivisions;
                triangle(anchor,pts[j]+glm::vec2(std::cos(t1),std::sin(t1))*half,pts[j]+glm::vec2(std::cos(t2),std::sin(t2))*half,u[i+1],-side,side,side);}
        } else triangle(anchor,pts[j]+from*half,pts[j]+to*half,u[i+1],-side,side,side);
    }
    return out;
}
}

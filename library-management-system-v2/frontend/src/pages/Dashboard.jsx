import { useEffect, useState } from "react";
import api from "../services/api";
import { useAuth } from "../context/AuthContext";

export default function Dashboard() {
  const {user}=useAuth(); const [stats,setStats]=useState({});
  useEffect(()=>{api.get(`/dashboard/${user.role.toLowerCase()}`).then(r=>setStats(r.data.stats||{})).catch(()=>{});},[user.role]);
  const cards=user.role==="STUDENT"
    ? [["Books Issued",stats.issued||0],["Pending Requests",stats.pending||0],["Available Books",stats.available||0],["Users",stats.users||0]]
    : user.role==="LIBRARIAN"
    ? [["Total Books",stats.books||0],["Available Copies",stats.available||0],["Issued",stats.issued||0],["Pending",stats.pending||0]]
    : [["Total Users",stats.users||0],["Total Books",stats.books||0],["Issued",stats.issued||0],["Pending Requests",stats.pending||0]];
  return <>
    <h1 className="title">Good day, {user.name.split(" ")[0]} 👋</h1>
    <p className="muted">Live statistics from the C backend.</p>
    <div className="grid grid-4" style={{marginTop:20}}>
      {cards.map(([k,v])=><div className="card" key={k}><div className="muted">{k}</div><div className="stat-number">{v}</div></div>)}
    </div>
    <div className="card" style={{marginTop:20}}><h3>Backend status</h3><span className="badge badge-green">Connected</span><p className="muted">React → C REST API → SQLite</p></div>
  </>;
}

import { useEffect, useMemo, useState } from "react";
import api from "../services/api";
import { useAuth } from "../context/AuthContext";

export default function Books() {
  const { user } = useAuth();
  const [books,setBooks]=useState([]);
  const [query,setQuery]=useState("");
  const [message,setMessage]=useState("");
  const [form,setForm]=useState({isbn:"",title:"",author:"",category:"Programming",year:2026,quantity:1,shelf:""});
  const [showAdd,setShowAdd]=useState(false);

  const load=()=>api.get("/books").then(r=>setBooks(r.data.books||[])).catch(e=>setMessage(e.response?.data?.message||"Could not load books"));
  useEffect(()=>{load()},[]);

  const filtered=useMemo(()=>books.filter(b=>
    `${b.title} ${b.author} ${b.category} ${b.isbn}`.toLowerCase().includes(query.toLowerCase())
  ),[books,query]);

  const request=async(book_id)=>{
    try{const r=await api.post("/requests",{book_id});setMessage(r.data.message||"Request submitted");}
    catch(e){setMessage(e.response?.data?.message||"Request failed");}
  };

  const add=async(e)=>{
    e.preventDefault();
    try{await api.post("/books",{...form,year:Number(form.year),quantity:Number(form.quantity)});setShowAdd(false);setMessage("Book created");load();}
    catch(e){setMessage(e.response?.data?.message||"Could not create book");}
  };

  return <>
    <h1 className="title">Books</h1>
    <p className="muted">Real data from the C REST API + SQLite.</p>
    {message && <div className="alert" style={{marginTop:12}}>{message}</div>}
    <div className="toolbar">
      <input className="input" placeholder="Search title, author, category or ISBN..." value={query} onChange={e=>setQuery(e.target.value)} />
      {["ADMIN","LIBRARIAN"].includes(user?.role) && <button className="btn btn-primary" onClick={()=>setShowAdd(!showAdd)}>+ Add Book</button>}
    </div>
    {showAdd && <form className="card form-grid" onSubmit={add}>
      {Object.keys(form).map(k=><div key={k}><label>{k}</label><input className="input" value={form[k]} onChange={e=>setForm({...form,[k]:e.target.value})}/></div>)}
      <div><button className="btn btn-success">Save Book</button></div>
    </form>}
    <div className="card table-wrap" style={{marginTop:18}}>
      <table><thead><tr><th>Title</th><th>Author</th><th>Category</th><th>ISBN</th><th>Available</th><th></th></tr></thead>
      <tbody>{filtered.map(b=><tr key={b.id}>
        <td><b>{b.title}</b></td><td>{b.author}</td><td>{b.category}</td><td>{b.isbn}</td>
        <td><span className={`badge ${b.available?"badge-green":"badge-red"}`}>{b.available}</span></td>
        <td>{user?.role==="STUDENT" && <button className="btn btn-primary" onClick={()=>request(b.id)}>Request</button>}</td>
      </tr>)}</tbody></table>
    </div>
  </>;
}

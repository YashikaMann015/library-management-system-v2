import { NavLink } from "react-router-dom";
import { BookOpen, LayoutDashboard, Users, ClipboardList, BookMarked, IndianRupee, BarChart3, Settings, LogOut } from "lucide-react";
import { useAuth } from "../context/AuthContext";

export default function Sidebar() {
  const { user, logout } = useAuth();
  const role = user?.role;

  const common = [
    ["/dashboard", "Dashboard", LayoutDashboard]
  ];
  const student = [
    ["/books", "Search Books", BookOpen],
    ["/my-books", "My Books", BookMarked],
    ["/requests", "Requests", ClipboardList],
    ["/fines", "My Fines", IndianRupee]
  ];
  const librarian = [
    ["/books", "Manage Books", BookOpen],
    ["/requests", "Requests", ClipboardList],
    ["/issues", "Issues & Returns", BookMarked],
    ["/fines", "Fines", IndianRupee],
    ["/students", "Students", Users],
    ["/reports", "Reports", BarChart3]
  ];
  const admin = [
    ["/users", "Users", Users],
    ["/books", "Books", BookOpen],
    ["/requests", "Requests", ClipboardList],
    ["/issues", "Issues", BookMarked],
    ["/fines", "Fines", IndianRupee],
    ["/reports", "Reports", BarChart3],
    ["/settings", "Settings", Settings]
  ];

  let items = common;
  if (role === "STUDENT") items = [...common, ...student];
  if (role === "LIBRARIAN") items = [...common, ...librarian];
  if (role === "ADMIN") items = [...common, ...admin];

  return (
    <aside className="sidebar">
      <div className="brand">📚 Library<span className="nav-label"> Management</span></div>
      {items.map(([to, label, Icon]) => (
        <NavLink key={to} to={to} className={({isActive}) => `nav-item ${isActive ? "active" : ""}`}>
          <Icon size={18} style={{verticalAlign:"middle", marginRight: 9}} />
          <span className="nav-label">{label}</span>
        </NavLink>
      ))}
      <button onClick={logout} className="nav-item" style={{width:"100%", background:"transparent", border:0, textAlign:"left", marginTop:20}}>
        <LogOut size={18} style={{verticalAlign:"middle", marginRight:9}} />
        <span className="nav-label">Logout</span>
      </button>
    </aside>
  );
}

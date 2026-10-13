import { Outlet } from "react-router-dom";
import Sidebar from "./Sidebar";
import { useAuth } from "../context/AuthContext";

export default function Layout() {
  const { user } = useAuth();
  return (
    <div className="shell">
      <Sidebar />
      <main className="main">
        <header className="topbar">
          <strong>Library Management System</strong>
          <span>{user?.name} · {user?.role}</span>
        </header>
        <section className="content"><Outlet /></section>
      </main>
    </div>
  );
}

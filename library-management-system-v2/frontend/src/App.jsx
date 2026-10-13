import { Routes, Route, Navigate } from "react-router-dom";
import ProtectedRoute from "./components/ProtectedRoute";
import Layout from "./components/Layout";
import Login from "./pages/Login";
import Dashboard from "./pages/Dashboard";
import Books from "./pages/Books";
import Placeholder from "./pages/Placeholder";

function P({title}) { return <Placeholder title={title}/>; }

export default function App() {
  return (
    <Routes>
      <Route path="/login" element={<Login />} />
      <Route element={<ProtectedRoute roles={["STUDENT","LIBRARIAN","ADMIN"]}><Layout/></ProtectedRoute>}>
        <Route path="/dashboard" element={<Dashboard/>} />
        <Route path="/books" element={<Books/>} />
        <Route path="/my-books" element={<P title="My Books"/>} />
        <Route path="/requests" element={<P title="Borrow Requests"/>} />
        <Route path="/fines" element={<P title="Fines"/>} />
        <Route path="/issues" element={<P title="Issues & Returns"/>} />
        <Route path="/students" element={<P title="Students"/>} />
        <Route path="/reports" element={<P title="Reports"/>} />
        <Route path="/users" element={<P title="User Management"/>} />
        <Route path="/settings" element={<P title="System Settings"/>} />
      </Route>
      <Route path="/" element={<Navigate to="/dashboard" replace />} />
      <Route path="*" element={<Navigate to="/dashboard" replace />} />
    </Routes>
  );
}

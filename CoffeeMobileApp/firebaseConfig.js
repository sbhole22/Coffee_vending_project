// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { getDatabase } from "firebase/database";

// TODO: Replace the following with your app's Firebase project configuration
// You can find this in the Firebase Console -> Project Settings -> General -> Your Apps -> SDK Setup and Configuration
const firebaseConfig = {
  apiKey: "<Your API Key here>",
  authDomain: "<Your Auth Domain here>",
  databaseURL: "<Your Database URL here>",
  projectId: "coffee-vending-f940c",
  storageBucket: "coffee-vending-f940c.firebasestorage.app",
  messagingSenderId: "714270016908",
  appId: "1:714270016908:web:40d4842ec3ed58fe74c2a9",
  measurementId: "G-XQTJSFJ442"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

export { database };

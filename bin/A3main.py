#!/usr/bin/env python3

import ctypes
import os
import sys
import sqlite3
import mysql.connector
from typing import List, Dict, Any, Optional

from asciimatics.widgets import (
    Frame,
    ListBox,
    Layout,
    Divider,
    Text,
    Button,
    Label,
    Widget,
    Text,
)
from asciimatics.scene import Scene
from asciimatics.screen import Screen
from asciimatics.exceptions import ResizeScreenError, NextScene, StopApplication

lib = ctypes.CDLL("./libvcparser.so")

lib.createCard.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_void_p)]
lib.createCard.restype = ctypes.c_int

lib.validateCard.argtypes = [ctypes.c_void_p]
lib.validateCard.restype = ctypes.c_int

lib.deleteCard.argtypes = [ctypes.c_void_p]
lib.createEmptyCard.restype = ctypes.c_void_p
lib.setFN.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
lib.setFN.restype = ctypes.c_int
lib.writeCard.argtypes = [ctypes.c_char_p, ctypes.c_void_p]
lib.writeCard.restype = ctypes.c_int

OK = 0
INV_FILE = 1
INV_CARD = 2
INV_PROP = 3
INV_DT = 4
WRITE_ERROR = 5
OTHER_ERROR = 6


class LoginView(Frame):
    def __init__(self, screen, model):
        super(LoginView, self).__init__(
            screen,
            screen.height * 2 // 3,
            screen.width * 2 // 3,
            hover_focus=True,
            can_scroll=False,
            title="Database Login",
            reduce_cpu=True,
        )

        self._model = model
        self._screen = screen

        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)

        self._error_label = Label("", align="^")
        layout.add_widget(self._error_label)

        layout.add_widget(Text("Username:", "username"))
        layout.add_widget(Text("Password:", "password", hide_char="*"))
        layout.add_widget(Text("Database Name:", "database"))

        layout2 = Layout([1, 1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("Login", self._login), 0)
        layout2.add_widget(Button("Cancel", self._quit), 2)

        self.fix()

    def _login(self):
        self.save()
        username = self.data.get("username", "")
        password = self.data.get("password", "")
        database = self.data.get("database", "")

        conn = self._connect_to_database(database, username, password)

        if conn:
            self._model.db_connection = conn

            try:
                self._model.create_database_tables()
            except Exception as e:
                conn.close()
                self._error_label.text = f"Table Creation Failed: {str(e)}"
                return

            self._error_label.text = ""
            raise NextScene("Main")
        else:
            self._error_label.text = "Login Failed. Check credentials and try again."

    def _connect_to_database(self, dbName, uName, passwd):
        try:
            conn = mysql.connector.connect(
                host="dursley.socs.uoguelph.ca",
                database=dbName,
                user=uName,
                password=passwd,
            )
            return conn
        except mysql.connector.Error as err:
            print(f"Database Connection Error: {err}")
            return None

    @staticmethod
    def _quit():
        raise StopApplication("User pressed quit")

    def reset(self):
        super(LoginView, self).reset()
        self.data = {"username": "", "password": "", "database": ""}
        self._error_label.text = ""


class ContactModel:
    def __init__(self):
        class DateTime(ctypes.Structure):
            _fields_ = [
                ("UTC", ctypes.c_bool),
                ("isText", ctypes.c_bool),
                ("date", ctypes.c_char_p),
                ("time", ctypes.c_char_p),
                ("text", ctypes.c_char_p),
            ]

        class Parameter(ctypes.Structure):
            _fields_ = [("name", ctypes.c_char_p), ("value", ctypes.c_char_p)]

        class Property(ctypes.Structure):
            _fields_ = [
                ("name", ctypes.c_char_p),
                ("group", ctypes.c_char_p),
                ("parameters", ctypes.c_void_p),
                ("values", ctypes.c_void_p),
            ]

        class Card(ctypes.Structure):
            _fields_ = [
                ("fn", ctypes.POINTER(Property)),
                ("optionalProperties", ctypes.c_void_p),
                ("birthday", ctypes.POINTER(DateTime)),
                ("anniversary", ctypes.POINTER(DateTime)),
            ]

        self.DateTime = DateTime
        self.Parameter = Parameter
        self.Property = Property
        self.Card = Card
        self.db_connection = None
        self.db_type = None
        self._sqlite_db = sqlite3.connect(":memory:")
        self._sqlite_db.row_factory = sqlite3.Row
        self._sqlite_db.cursor().execute(
            """
            CREATE TABLE contacts(
                id INTEGER PRIMARY KEY,
                file_name TEXT,
                contact_name TEXT,
                birthday TEXT,
                anniversary TEXT,
                other_properties TEXT)
        """
        )
        self._sqlite_db.commit()
        self.current_id = None
        self.populate_vcards()

    def set_database_connection(self, connection, db_type="mysql"):
        self.db_connection = connection
        self.db_type = db_type

    def execute_query(self, query: str, params: tuple = None, fetch: str = None) -> Any:
        if not self.db_connection:
            raise RuntimeError("No database connection established")

        cursor = self.db_connection.cursor(dictionary=True)

        try:
            if params:
                cursor.execute(query, params)
            else:
                cursor.execute(query)

            if fetch == "one":
                return cursor.fetchone()
            elif fetch == "all":
                return cursor.fetchall()
            else:
                self.db_connection.commit()
                return None
        except mysql.connector.Error as err:
            print(f"Database Error: {err}")
            return None
        finally:
            cursor.close()

    def populate_vcards(self, directory: str = "cards"):
        if not os.path.exists(directory):
            return

        lib = ctypes.CDLL("./libvcparser.so")
        lib.getOtherPropertiesCount.argtypes = [ctypes.POINTER(self.Card)]
        lib.getOtherPropertiesCount.restype = ctypes.c_int

        for filename in os.listdir(directory):
            if filename.endswith(".vcf"):
                file_path = os.path.join(directory, filename)

                card_ptr = ctypes.c_void_p()

                result = lib.createCard(
                    file_path.encode("utf-8"), ctypes.byref(card_ptr)
                )

                if result == 0:
                    validate_result = lib.validateCard(card_ptr)

                    if validate_result == 0:
                        card = ctypes.cast(card_ptr, ctypes.POINTER(self.Card)).contents

                        contact_name = ""
                        if card.fn:
                            lib.getFirstValue.argtypes = [ctypes.c_void_p]
                            lib.getFirstValue.restype = ctypes.c_char_p
                            contact_name_bytes = lib.getFirstValue(
                                card.fn.contents.values
                            )
                            if contact_name_bytes:
                                contact_name = contact_name_bytes.decode("utf-8")

                        if not contact_name:
                            contact_name = filename.replace(".vcf", "")

                        birthday = self.parse_vcard_date(card.birthday)
                        anniversary = self.parse_vcard_date(card.anniversary)

                        other_properties_count = lib.getOtherPropertiesCount(
                            ctypes.byref(card)
                        )

                        contact = {
                            "file_name": filename,
                            "contact_name": contact_name,
                            "birthday": birthday,
                            "anniversary": anniversary,
                            "other_properties": f"{other_properties_count} ",
                        }
                        self.add(contact)

                    lib.deleteCard(card_ptr)

    def add(self, contact: Dict[str, Any]) -> int:
        cursor = self._sqlite_db.cursor()
        cursor.execute(
            """
            INSERT INTO contacts(file_name, contact_name, birthday, anniversary, other_properties)
            VALUES(:file_name, :contact_name, :birthday, :anniversary, :other_properties)""",
            contact,
        )
        self._sqlite_db.commit()
        sqlite_id = cursor.lastrowid

        if self.db_connection and self.db_type == "mysql":
            try:
                query = """
                INSERT INTO contacts 
                (file_name, contact_name, birthday, anniversary, other_properties) 
                VALUES (%s, %s, %s, %s, %s)
                """
                params = (
                    contact["file_name"],
                    contact["contact_name"],
                    contact["birthday"],
                    contact["anniversary"],
                    contact["other_properties"],
                )
                self.execute_query(query, params)
            except Exception as e:
                print(f"Error syncing to MySQL: {e}")

        return sqlite_id

    def get_contact(self, contact_id: int) -> Dict[str, Any]:
        if self.db_connection and self.db_type == "mysql":
            query = "SELECT * FROM contacts WHERE id = %s"
            result = self.execute_query(query, (contact_id,), fetch="one")
            if result:
                return result

        contact_dict = (
            self._sqlite_db.cursor()
            .execute("SELECT * from contacts WHERE id=:id", {"id": contact_id})
            .fetchone()
        )

        if contact_dict:
            contact_dict = dict(contact_dict)
            contact_dict["other_properties"] = contact_dict.get(
                "other_properties", "No additional properties"
            )

        return contact_dict

    def get_summary(self) -> List[Dict[str, Any]]:
        if self.db_connection and self.db_type == "mysql":
            query = "SELECT file_name, id FROM contacts"
            result = self.execute_query(query, fetch="all")
            if result:
                return result

        return (
            self._sqlite_db.cursor()
            .execute("SELECT file_name, id from contacts")
            .fetchall()
        )

    def update_current_contact(self, details: Dict[str, Any]) -> None:
        self._sqlite_db.cursor().execute(
            """
            UPDATE contacts SET file_name=:file_name, contact_name=:contact_name, 
            birthday=:birthday, anniversary=:anniversary, other_properties=:other_properties 
            WHERE id=:id""",
            details,
        )
        self._sqlite_db.commit()

        if self.db_connection and self.db_type == "mysql":
            query = """
            UPDATE contacts 
            SET file_name = %s, contact_name = %s, 
            birthday = %s, anniversary = %s, other_properties = %s 
            WHERE id = %s
            """
            params = (
                details["file_name"],
                details["contact_name"],
                details["birthday"],
                details["anniversary"],
                details["other_properties"],
                details["id"],
            )
            self.execute_query(query, params)

    def parse_vcard_date(self, date_ptr) -> str:
        if date_ptr is None or not bool(date_ptr):
            return ""

        try:
            dt = ctypes.cast(date_ptr, ctypes.POINTER(self.DateTime)).contents

            if dt.isText and dt.text:
                return dt.text.decode("utf-8")

            result_parts = []

            if dt.date and dt.date.decode("utf-8").strip():
                result_parts.append(f"Date: {dt.date.decode('utf-8')}")

            if dt.time and dt.time.decode("utf-8").strip():
                result_parts.append(f"Time: {dt.time.decode('utf-8')}")

            if dt.UTC:
                result_parts.append("(UTC)")

            return " ".join(result_parts) if result_parts else ""
        except Exception:
            return ""

    def get_current_contact(self) -> Dict[str, Any]:
        if self.current_id is None:
            return {
                "file_name": "",
                "contact_name": "",
                "birthday": "",
                "anniversary": "",
                "other_properties": "",
            }
        else:
            return self.get_contact(self.current_id)

    def create_database_tables(self):
        if not self.db_connection:
            raise RuntimeError("No database connection established")

        try:
            cursor = self.db_connection.cursor()

            cursor.execute("DROP TABLE IF EXISTS CONTACT")
            cursor.execute("DROP TABLE IF EXISTS FILE")

            cursor.execute(
                """
            CREATE TABLE FILE (
                id INTEGER AUTO_INCREMENT PRIMARY KEY,
                file_name VARCHAR(255) NOT NULL,
                file_path VARCHAR(512),
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
            """
            )

            cursor.execute(
                """
            CREATE TABLE CONTACT (
                id INTEGER AUTO_INCREMENT PRIMARY KEY,
                file_id INTEGER,
                first_name VARCHAR(255),
                last_name VARCHAR(255),
                birthday DATE,
                anniversary DATE,
                other_properties TEXT,
                FOREIGN KEY (file_id) REFERENCES FILE(id)
            )
            """
            )

            self.db_connection.commit()
            print("Tables FILE and CONTACT created")

            self.populate_mysql_tables()

        except mysql.connector.Error as err:
            print(f"Error creating tables: {err}")
            raise

    def populate_mysql_tables(self, directory: str = "cards"):
        if not os.path.exists(directory):
            return

        lib = ctypes.CDLL("./libvcparser.so")
        lib.getOtherPropertiesCount.argtypes = [ctypes.POINTER(self.Card)]
        lib.getOtherPropertiesCount.restype = ctypes.c_int

        cursor = self.db_connection.cursor()

        for filename in os.listdir(directory):
            if filename.endswith(".vcf"):
                file_path = os.path.join(directory, filename)

                card_ptr = ctypes.c_void_p()

                result = lib.createCard(
                    file_path.encode("utf-8"), ctypes.byref(card_ptr)
                )

                if result == 0:
                    validate_result = lib.validateCard(card_ptr)

                    if validate_result == 0:
                        card = ctypes.cast(card_ptr, ctypes.POINTER(self.Card)).contents

                        contact_name = ""
                        if card.fn:
                            lib.getFirstValue.argtypes = [ctypes.c_void_p]
                            lib.getFirstValue.restype = ctypes.c_char_p
                            contact_name_bytes = lib.getFirstValue(
                                card.fn.contents.values
                            )
                            if contact_name_bytes:
                                contact_name = contact_name_bytes.decode("utf-8")

                        if not contact_name:
                            contact_name = filename.replace(".vcf", "")

                        name_parts = contact_name.split(" ", 1)
                        first_name = name_parts[0]
                        last_name = name_parts[1] if len(name_parts) > 1 else ""

                        def parse_vcard_datetime(date_ptr):
                            if date_ptr is None or not bool(date_ptr):
                                return None

                            dt = ctypes.cast(
                                date_ptr, ctypes.POINTER(self.DateTime)
                            ).contents

                            if dt.isText and dt.text:
                                return None

                            if not dt.date:
                                return None

                            date_str = dt.date.decode("utf-8").strip()

                            if len(date_str) != 8 or not date_str.isdigit():
                                return None

                            return f"{date_str[:4]}-{date_str[4:6]}-{date_str[6:]}"

                        birthday = parse_vcard_datetime(card.birthday)
                        anniversary = parse_vcard_datetime(card.anniversary)

                        other_properties_count = lib.getOtherPropertiesCount(
                            ctypes.byref(card)
                        )

                        file_insert_query = """
                        INSERT INTO FILE (file_name, file_path) 
                        VALUES (%s, %s)
                        """
                        cursor.execute(file_insert_query, (filename, file_path))
                        file_id = cursor.lastrowid

                        contact_insert_query = """
                        INSERT INTO CONTACT 
                        (file_id, first_name, last_name, birthday, anniversary, other_properties) 
                        VALUES (%s, %s, %s, %s, %s, %s)
                        """
                        cursor.execute(
                            contact_insert_query,
                            (
                                file_id,
                                first_name,
                                last_name,
                                birthday,
                                anniversary,
                                str(other_properties_count),
                            ),
                        )

                    lib.deleteCard(card_ptr)

        self.db_connection.commit()
        print(f"Populated MySQL tables with vCards from {directory}")


class ListView(Frame):
    def __init__(self, screen, model):
        super(ListView, self).__init__(
            screen,
            screen.height * 2 // 3,
            screen.width * 2 // 3,
            on_load=self._reload_list,
            hover_focus=True,
            can_scroll=False,
            title="vCard List",
        )

        self._model = model

        self._list_view = ListBox(
            Widget.FILL_FRAME,
            model.get_summary(),
            name="contacts",
            add_scroll_bar=True,
            on_change=self._on_pick,
            on_select=self._edit,
        )
        self._edit_button = Button("Edit", self._edit)

        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)
        layout.add_widget(self._list_view)
        layout.add_widget(Divider())

        layout2 = Layout([1, 1, 1, 1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("Create", self._add), 0)
        layout2.add_widget(self._edit_button, 1)
        layout2.add_widget(Button("DB Queries", self._db_queries), 2)
        layout2.add_widget(Button("Cancel", self._quit), 4)

        self.fix()
        self._on_pick()

    def _db_queries(self):
        raise NextScene("DB Queries")

    def _on_pick(self):
        self._edit_button.disabled = self._list_view.value is None

    def _reload_list(self, new_value=None):
        self._list_view.options = self._model.get_summary()
        self._list_view.value = new_value

    def _add(self):
        self._model.current_id = None
        raise NextScene("Edit Contact")

    def _edit(self):
        self.save()
        self._model.current_id = self.data["contacts"]
        raise NextScene("Edit Contact")

    @staticmethod
    def _quit():
        raise StopApplication("User pressed quit")


class ContactView(Frame):
    def __init__(self, screen, model):
        super(ContactView, self).__init__(
            screen,
            screen.height * 2 // 3,
            screen.width * 2 // 3,
            hover_focus=True,
            can_scroll=False,
            title="Contact Details",
            reduce_cpu=True,
        )

        self._model = model

        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)

        layout.add_widget(Text("File name:", "file_name"))
        layout.add_widget(Text("Contact name:", "contact_name"))

        layout.add_widget(Text("Birthday:", "birthday", disabled=True))
        layout.add_widget(Text("Anniversary:", "anniversary", disabled=True))
        layout.add_widget(Text("Other Properties:", "other_properties", disabled=True))

        layout2 = Layout([1, 1, 1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("OK", self._ok), 0)
        layout2.add_widget(Button("Cancel", self._cancel), 3)

        self.fix()

    def reset(self):
        super(ContactView, self).reset()
        self.data = self._model.get_current_contact()

    def _ok(self):
        self.save()
        file_name = self.data["file_name"]
        contact_name = self.data["contact_name"]

        if self._model.current_id is None:
            card_ptr = lib.createEmptyCard()
            if not card_ptr:
                self._show_error("Error: Could not create vCard")
                return

            if lib.setFN(card_ptr, contact_name.encode("utf-8")) != OK:
                lib.deleteCard(card_ptr)
                self._show_error("Error: Could not set Full Name")
                return

            if (
                lib.validateCard(card_ptr) == OK
                and lib.writeCard(f"cards/{file_name}".encode("utf-8"), card_ptr) == OK
            ):
                name_parts = contact_name.split(" ", 1)
                first_name = name_parts[0]
                last_name = name_parts[1] if len(name_parts) > 1 else ""

                new_contact = {
                    "file_name": file_name,
                    "contact_name": contact_name,
                    "first_name": first_name,
                    "last_name": last_name,
                    "birthday": "",
                    "anniversary": "",
                    "other_properties": "0",
                }

                new_id = self._model.add(new_contact)

                if self._model.db_connection:
                    try:
                        cursor = self._model.db_connection.cursor()

                        file_insert_query = """
                        INSERT INTO FILE (file_name, file_path) 
                        VALUES (%s, %s)
                        """
                        cursor.execute(
                            file_insert_query, (file_name, f"cards/{file_name}")
                        )
                        file_id = cursor.lastrowid

                        contact_insert_query = """
                        INSERT INTO CONTACT 
                        (file_id, first_name, last_name, birthday, anniversary, other_properties) 
                        VALUES (%s, %s, %s, %s, %s, %s)
                        """
                        cursor.execute(
                            contact_insert_query,
                            (
                                file_id,
                                first_name,
                                last_name,
                                None,
                                None,
                                "0",
                            ),
                        )

                        self._model.db_connection.commit()
                    except mysql.connector.Error as err:
                        print(f"MySQL Insert Error: {err}")

            lib.deleteCard(card_ptr)
        else:
            existing_contact = self._model.get_current_contact()
            original_file_path = f"cards/{existing_contact['file_name']}"
            card_ptr = ctypes.c_void_p()

            if (
                lib.createCard(
                    original_file_path.encode("utf-8"), ctypes.byref(card_ptr)
                )
                == OK
            ):
                lib.setFN(card_ptr, contact_name.encode("utf-8"))
                if (
                    lib.validateCard(card_ptr) == OK
                    and lib.writeCard(f"cards/{file_name}".encode("utf-8"), card_ptr)
                    == OK
                ):
                    name_parts = contact_name.split(" ", 1)
                    first_name = name_parts[0]
                    last_name = name_parts[1] if len(name_parts) > 1 else ""

                    updated_contact = {
                        "id": existing_contact["id"],
                        "file_name": file_name,
                        "contact_name": contact_name,
                        "first_name": first_name,
                        "last_name": last_name,
                        "birthday": existing_contact["birthday"],
                        "anniversary": existing_contact["anniversary"],
                        "other_properties": existing_contact["other_properties"],
                    }

                    self._model.update_current_contact(updated_contact)

                    if self._model.db_connection:
                        try:
                            cursor = self._model.db_connection.cursor()

                            file_update_query = """
                            UPDATE FILE 
                            SET file_name = %s, file_path = %s 
                            WHERE id IN (
                                SELECT file_id FROM CONTACT 
                                WHERE id = %s
                            )
                            """
                            cursor.execute(
                                file_update_query,
                                (
                                    file_name,
                                    f"cards/{file_name}",
                                    existing_contact["id"],
                                ),
                            )

                            contact_update_query = """
                            UPDATE CONTACT 
                            SET first_name = %s, last_name = %s
                            WHERE id = %s
                            """
                            cursor.execute(
                                contact_update_query,
                                (first_name, last_name, existing_contact["id"]),
                            )

                            self._model.db_connection.commit()
                        except mysql.connector.Error as err:
                            print(f"MySQL Update Error: {err}")

                else:
                    self._show_error("Error: Invalid vCard")

                lib.deleteCard(card_ptr)

        raise NextScene("Main")

    def _cancel(self):
        raise NextScene("Main")


class DBQueriesView(Frame):
    def __init__(self, screen, model):
        super(DBQueriesView, self).__init__(
            screen,
            screen.height * 2 // 3,
            screen.width * 2 // 3,
            hover_focus=True,
            can_scroll=False,
            title="Database Queries",
            reduce_cpu=True,
        )

        self._model = model
        self._screen = screen

        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)

        self._status_label = Label("", align="^")
        layout.add_widget(self._status_label)

        self._results_view = ListBox(
            Widget.FILL_FRAME,
            [],
            name="query_results",
            add_scroll_bar=True,
        )
        layout.add_widget(self._results_view)
        layout.add_widget(Divider())

        layout2 = Layout([1, 1, 1])
        self.add_layout(layout2)
        layout2.add_widget(
            Button("Display all contacts", self._display_all_contacts), 0
        )
        layout2.add_widget(
            Button("Find contacts born in June", self._find_born_in_june), 1
        )
        layout2.add_widget(Button("Cancel", self._cancel), 2)

        self.fix()

    def _display_all_contacts(self):
        self._status_label.text = ""
        self._results_view.options = []

        if not self._model.db_connection:
            self._status_label.text = "Error: No database connection"
            return

        try:
            cursor = self._model.db_connection.cursor(dictionary=True)

            query = """
            SELECT 
                c.id, 
                c.first_name, 
                c.last_name, 
                c.birthday, 
                c.anniversary, 
                c.other_properties
            FROM 
                CONTACT c
            ORDER BY 
                c.first_name, c.last_name
            """

            cursor.execute(query)
            results = cursor.fetchall()

            if not results:
                self._results_view.options = [("No contacts found", None)]
                return

            display_options = []
            for contact in results:
                contact_str = (
                    f"Name: {contact.get('first_name', '')} {contact.get('last_name', '')} | "
                    f"Birthday: {contact.get('birthday', 'N/A')} | "
                    f"Anniversary: {contact.get('anniversary', 'N/A')} | "
                    f"Other Props: {contact.get('other_properties', 'N/A')}"
                )
                display_options.append((contact_str, contact.get("id")))

            self._results_view.options = display_options
            self._results_view.value = (
                display_options[0][1] if display_options else None
            )

            self._status_label.text = f"Found {len(display_options)} contacts"

        except mysql.connector.Error as err:
            self._status_label.text = f"MySQL Error: {err}"
            print(f"Full MySQL Error Details: {err}")
            self._results_view.options = [(f"Database Error: {err}", None)]
        except Exception as e:
            self._status_label.text = f"Unexpected Error: {e}"
            print(f"Full Error Details: {e}")
            self._results_view.options = [(f"Unexpected Error: {e}", None)]

    def _find_born_in_june(self):
        self._status_label.text = ""
        self._results_view.options = []

        if not self._model.db_connection:
            self._status_label.text = "Error: No database connection"
            return

        try:
            cursor = self._model.db_connection.cursor(dictionary=True)

            query = """
            SELECT 
                c.id, 
                c.first_name, 
                c.last_name, 
                c.birthday,
                f.file_name,
                f.created_at
            FROM 
                CONTACT c
            JOIN 
                FILE f ON c.file_id = f.id
            WHERE 
                MONTH(c.birthday) = 6
            ORDER BY 
                c.birthday
            """

            cursor.execute(query)
            results = cursor.fetchall()

            if not results:
                self._results_view.options = [("No contacts born in June found", None)]
                return

            display_options = []
            from datetime import datetime, date

            for contact in results:
                full_name = (
                    f"{contact.get('first_name', '')} {contact.get('last_name', '')}"
                )

                birthday = contact.get("birthday")

                if birthday and contact.get("created_at"):
                    try:
                        birth_date = datetime.strptime(str(birthday), "%Y-%m-%d").date()

                        creation_time = contact.get("created_at")

                        age = creation_time.year - birth_date.year

                        if (creation_time.month, creation_time.day) < (
                            birth_date.month,
                            birth_date.day,
                        ):
                            age -= 1

                        contact_str = (
                            f"Name: {full_name} | "
                            f"Birthday: {birthday} | "
                            f"Age: {age}"
                        )
                        display_options.append((contact_str, contact.get("id")))
                    except Exception as e:
                        print(f"Error processing contact {full_name}: {e}")

            display_options.sort(
                key=lambda x: int(x[0].split("Age: ")[1]) if "Age: " in x[0] else 0
            )

            self._results_view.options = display_options
            self._results_view.value = (
                display_options[0][1] if display_options else None
            )

            self._status_label.text = (
                f"Found {len(display_options)} contacts born in June"
            )

        except mysql.connector.Error as err:
            self._status_label.text = f"MySQL Error: {err}"
            print(f"Full MySQL Error Details: {err}")
            self._results_view.options = [(f"Database Error: {err}", None)]
        except Exception as e:
            self._status_label.text = f"Unexpected Error: {e}"
            print(f"Full Error Details: {e}")
            self._results_view.options = [(f"Unexpected Error: {e}", None)]

    @staticmethod
    def _cancel():
        raise NextScene("Main")


def demo(screen, scene):
    scenes = [
        Scene([LoginView(screen, contacts)], -1, name="Login"),
        Scene([ListView(screen, contacts)], -1, name="Main"),
        Scene([ContactView(screen, contacts)], -1, name="Edit Contact"),
        Scene([DBQueriesView(screen, contacts)], -1, name="DB Queries"),
    ]

    start_scene = scene if scene is not None else scenes[0]

    screen.play(scenes, stop_on_resize=True, start_scene=start_scene, allow_int=True)


contacts = ContactModel()
last_scene = None
while True:
    try:
        Screen.wrapper(demo, catch_interrupt=True, arguments=[last_scene])
        sys.exit(0)
    except ResizeScreenError as e:
        last_scene = e.scene
